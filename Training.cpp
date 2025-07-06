#include "JetStory.h"

#define IMAGECLASS OtherImg
#define IMAGEFILE <JetStory/Other.iml>
#include <drx/Draw/iml_source.h>

bool training;
int  training_counter;
int  training_remains;
int  training_total;

Vec<Rect> training_block;

tukk training_step[] {
	"[@G=/* Приступим к общей боевой подготовке!&]Используй клавишу #LEFT, чтобы пройти влево к КТ (контрольной точке).",

	"[@G/* Отлично!&]Теперь идём к другой КТ, используя клавиши #UP, #RIGHT, #LEFT!",

	"[@G/* Классно!&]Пожалуйста, навигируй в лабиринт!",

	"[@G/* Замечательно!&] Двигайся по лабиринту к следщ. КТ.&[/@R Осторожно, там теперь мины, которые тебя мигом убьют!]",

	"[@G/* Отличная работа!&]А теперь трудная часть.&[/@R Осторожнее, будет очень жарко!]",

	"[@G/* Великолепно!&]Ну, тогда пройди остаток лабиринта...",
	
	"[@G/* Выполнено неплохо!&]Всё это маневрирование по лабиринту стоило много топлива.&Подзаправься-ка, подобрав топливные канистры.&[/@R Когда кончится топливо, тебя ждёт неминуемвя смерть!]",

	"[@G/* Ну, смак же!&]Теперь уже можно начинать стрельбища.& Подбери снаряды и поражай цели клавишей #GUN!&"
	"[/@R ВНИМАНИЕ: столкновение с практическими целями уничтожит тебя в один миг...",

	"[@G/* Крутая стрельба!&]Порази же остальные три цели!!!",

	"[@G/* Крутая стрельба!&]]Порази же остальные две цели!!",

	"[@G/* Крутая стрельба!&]]Уничтожь оставшуюся цель!",

	"[@G/* Крутая стрельба!&]Ну, а теперь приступай к тренировке с запуском ракет.",
	"Некоторые цели укреплены щитами и рушатся только после многих попаданий из оружия ИЛИ "
	"от поражения мощными ракетами.&"
	"Теперь подбирай ракеты и поражай цели в этой зоне, используя клавишу #MISSILE.",

	"[@G/*  Крутая стрельба!&]Пожалуйста, уничтожь оставшиеся две цели!",

	"[@G/*  Крутая стрельба!&]Пожалуйста, уничтожь оставшуюся цель!",

	"[@G/* Крутая стрельба!&]Ну, а сейчас начинается тренировка с бомбами.",
	"Если траектория ракет неподходящая, а цель под тобой, тогда используй бомбы.&"
	"Теперь возьми запас бомб и порази цели, используя клавишу #BOMB.",

	"[@G/*  Крутая бомбардировка!&]Подай-ка жарку оставшейся мишени!",

	"[@G/* Поздравлёсы!&]Вы освоили курс начальной боевой подготовки. Соберись-ка и задай им жару!",
};

struct TPoint : Enemy {
	int index = 0;
	
	virtual bool IsActive() const { return index == TrainingIndex(); }

	virtual void SerializeKind(Stream& s) { s % index; }
	virtual Size GetSize() {
		return IsActive() ? image.GetSize() : Size(0, 0);
	}
	virtual void Paint(Point sp) {
		if(IsActive() || design_mode) {
			Enemy::Paint(sp);
			int n = min((AniTick() >> 4) % 12, 10);
			Image m = OtherImg::TArrow();
			Size isz = m.GetSize();
			int x1 = sp.x + n;
			int y1 = sp.y + n;
			int x2 = sp.x + 64 - n - isz.cx;
			int y2 = sp.y + 64 - n - isz.cy;
			DrawImage(x1, y1, m);
			DrawImage(x2, y1, m, SWAPX);
			DrawImage(x1, y2, m, SWAPY);
			DrawImage(x2, y2, m, SWAPX|SWAPY);
		}
		else
			DrawImage(sp.x, sp.y, OtherImg::TPointBlocking());
	}

	virtual void Do() { training_counter++; if(!IsActive()) training_block.Add(GetRect().Inflated(2)); }
	TPoint() { index = training_counter++; global = true; image = OtherImg::TPoint(); damage = TRAININGPOINT; z = 1; }
};

REGISTER(TPoint);

struct TTarget : Enemy {
	virtual void SerializeKind(Stream& s) {}
	virtual void Do() {
		training_counter++;
		image = (AniTick() >> 6) & 1 ? OtherImg::Target1() : OtherImg::Target2();
	}
	TTarget() { shield = 1; explosion_size = 20; global = true; damage = TRAININGTARGET; training_counter++; }
};

REGISTER(TTarget);

struct HTarget : Enemy {
	virtual void SerializeKind(Stream& s) {}
	virtual void Do() {
		training_counter++;
		image = (AniTick() >> 6) & 1 ? OtherImg::HardTarget1() : OtherImg::HardTarget2();
	}
	HTarget() {
		damage = TRAININGTARGET;
		training_counter++;
	    shield = 50;
		explosion_size = 150;
		global = true;
	}
};

REGISTER(HTarget);

struct TMine : Enemy {
	virtual void Do() {}
	virtual void SerializeKind(Stream& s) {}
	TMine() { image = OtherImg::Mine(); global = true; shield = explosion_size = 150000; damage = 1000; z = 2; }
};

REGISTER(TMine);

Image message_image;

void SetMessage(const Txt& msg, bool fl)
{
	static Txt message;
	if(msg != message) {
		message = msg;
		if(IsNull(msg))
			message_image = Null;
		else {
			RichText txt = ParseQTF(Txt("[@W= ") + message);
			txt.ApplyZoom(Zoom(3, 10));
			int cx = min(txt.GetWidth(), window_size.cx - 16);
			ImageBuffer ib(cx + 16, txt.GetHeight(cx) + 16);
			{
				BufferPainter w(ib);
				w.Clear(RGBAZero());
				txt.Paint(w, 8, 8, cx);
			}
			message_image = ib;
			flash = fl * 255;
		}
	}
}

void PaintMessage()
{
	if(gameover)
		return;
	static int ri = -1;
	if(ship.pos.y < 0) {
		bool b = ship.pos.x < -2000 || ship.pos.x > MAPX * 64 + 2000;
		static bool bb;
		if(bb != b) {
			bb = b;
			if(b)
				SetMessage("Наши сенсоры не обнаружили никаких баз в этом направлении.&Возвращайся, пока ещё есть горючка...", false);
			else
				SetMessage(Null);
		}
		ri = -1;
	}
	else
	if(training && !design_mode) {
		if(TrainingIndex() < __countof(training_step)) {
			if(ri != TrainingIndex()) {
				ri = TrainingIndex();
				VecMap<Txt, Txt> repl;
				static tukk k[] = { "#LEFT", "#RIGHT", "#UP", "#BOMB", "#GUN", "#MISSILE" };
				for(int i = 0; i < 6; i++)
					repl.Add(k[i], Txt() << "[@W$k  "
					               << SDL_GetScancodeName((SDL_Scancode)game_key[i])
					               << " ]");
				SetMessage(Replace(training_step[ri], repl), true);
			}
		}
		else
			SetMessage(Null);
	}
	if(!IsNull(message_image)) {
		Size isz = message_image.GetSize();
		DrawRect(Rect(Point((window_size.cx - isz.cx) / 2, window_size.cy - isz.cy),
		         message_image.GetSize()), 0.7 + 0.3 * flash / 255.0,
		         Blend(Blue(), Yellow(), flash));
		DrawImage((window_size.cx - isz.cx) / 2, window_size.cy - isz.cy, message_image);
		if(flash > 0)
			flash =  max(flash - 5, 0);
	}
}

void TrainingPoint()
{
	if(gameover)
		return;
//	StartMusic(TUNE_BASE);
	PlaySound(5, 0.5, "261.62:L20V100R35T::::");
	Checkpoint();
}
