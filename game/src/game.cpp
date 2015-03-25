// game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "board.h"
#include "conf.h"
#include "BoardCtrl.h"

typedef std::vector<SDL_Texture*> Textures;

namespace Guards
{
   template<class Type, void (*Del)(Type*)>
   class Base
   {
   public:
      Base(Type* obj):obj_(obj){}
      ~Base() {Free();}
      void Free() 
         {
            if(obj_)
               Del(obj_);
            obj_ = 0;
         }
      Type* obj_;
   };
   typedef Base<TTF_Font, TTF_CloseFont> Font;
   typedef Base<SDL_Surface, SDL_FreeSurface> Surface;
   typedef Base<SDL_Texture, SDL_DestroyTexture> Texture;
};

void ApplySurface(int x, int y, SDL_Texture *tex, SDL_Renderer *rend, double factor = 1)
{
	SDL_Rect pos;
	pos.x = x;
	pos.y = y;
	SDL_QueryTexture(tex, NULL, NULL, &pos.w, &pos.h);
    pos.x += static_cast<int>(pos.w - pos.w * factor) / 2;
    pos.w = static_cast<int>(pos.w * factor);
    pos.y += static_cast<int>(pos.h - pos.h * factor) / 2;
    pos.h = static_cast<int>(pos.h * factor);
	if(SDL_RenderCopy(rend, tex, NULL, &pos) < 0)
		throw std::runtime_error(SDL_GetError());
} 

Textures PrepareGemsTextures(const std::vector<std::string>& gems_res, SDL_Renderer* ren) 
{
	Textures gems;
	for(Textures::size_type i = 0; i < gems_res.size(); i++)
	{
      Guards::Surface image = IMG_Load(gems_res[i].c_str());
		if (image.obj_ == 0) 
			throw std::runtime_error(SDL_GetError());
		SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, image.obj_);
		if (tex == 0)
			throw std::runtime_error(SDL_GetError());
		gems.push_back(tex);
	}	
	return gems;
}

class GameDrawer
{
public:
	GameDrawer(SDL_Renderer* ren, SDL_Surface* bmp, 
		const std::vector<std::string>& gems_res, const std::string& font_file)
		:main_font(TTF_OpenFont( font_file.c_str(), 16 ))

	{
		if(!main_font.obj_)
			throw std::runtime_error(SDL_GetError());

		help.push_back(make_font_texture("  HELP:", ren));
		help.push_back(make_font_texture("Press 'Space' to start", ren));
		help.push_back(make_font_texture("'Left click' to select", ren));
		help.push_back(make_font_texture("             and swap", ren));
		help.push_back(make_font_texture("  RULES:", ren));
		help.push_back(make_font_texture(" Align 3 or more gems", ren));
		help.push_back(make_font_texture("horizontally or vertically", ren));
		help.push_back(make_font_texture("by swaping them", ren));

		gems = PrepareGemsTextures(gems_res, ren);
		SDL_QueryTexture(gems[0], NULL, NULL, &gem_width, &gem_height);

		background = SDL_CreateTextureFromSurface(ren, bmp);
	}
	void DrawHelp(int x, int y, SDL_Renderer* ren)
	{
      SDL_Rect pos = {x, y, 0, 0};
      const int interline = 2;
		for(Textures::size_type i = 0; i < help.size(); i++)
		{
         int w, h;
			SDL_QueryTexture(help[i], NULL, NULL, &w, &h);
			pos.h += h + interline;
         pos.w = std::max(w, pos.w);
		}
      const int margin = 5;
      pos.w += margin * 2;
      pos.h += margin * 2;
      DrawFrame(pos, ren, margin);
      int ypos = y + margin;
      int xpos = x + margin;
		for(Textures::size_type i = 0; i < help.size(); i++)
		{
			SDL_Rect pos;
			pos.x = xpos;
			pos.y = ypos;
			SDL_QueryTexture(help[i], NULL, NULL, &pos.w, &pos.h);
			SDL_RenderCopy(ren, help[i], NULL, &pos);
			ypos += pos.h + interline;
		}
	}
	void DrawBackground(SDL_Renderer* ren)
	{
		ApplySurface(0, 0, background, ren);
	}
	void RenderBoard(int x, int y, const Board& b, 
		int sel_r, int sel_c,
		SDL_Renderer* ren)
	{
		const int w = gem_width; 
		const int h = gem_height;
		SDL_Rect clip_rc = {x, y, w * b.Col(), h * b.Row()};
		if(SDL_RenderSetClipRect(ren, &clip_rc) != 0)
			throw std::runtime_error(SDL_GetError());
		for(int r = 1 - b.Row() ; r < b.Row() + 1; r++)
		{
			for(int c = 0; c < b.Col(); c++)
			{
				const Type cell = b.Get(r, c);
				int xpos = x + w * c;
				int ypos = y + h * r;
				switch(cell.state)
				{
				case GS_NONE:
					break;
				case GS_SWAP_HOR:
				case GS_SWAP_BACK_HOR:
					xpos += static_cast<int>(w * cell.offset);
					break;
				case GS_SWAP_VER:
				case GS_SWAP_BACK_VER:
				case GS_FALLING:
					ypos += static_cast<int>(h * cell.offset);
					break;
				case GS_BURNED:
				case GS_BORDER:
				case GS_HIDDEN:
				case GS_EMPTY:
					continue;
				}
				if(r == sel_r && c == sel_c)
				{
					const SDL_Rect rc1 = {xpos, ypos, w / 10, h};
					const SDL_Rect rc2 = {xpos + w - w/10 , ypos, w / 10, h};
					const SDL_Rect rc3 = {xpos, ypos, w, h / 10};
					const SDL_Rect rc4 = {xpos, ypos + h - h /10, w, h / 10};
					SDL_SetRenderDrawColor(ren, 0xff, 0, 0, 0xff);
					if(SDL_RenderFillRect(ren, &rc1) < 0 || 
						SDL_RenderFillRect(ren, &rc2) < 0 ||
						SDL_RenderFillRect(ren, &rc3) < 0 ||
						SDL_RenderFillRect(ren, &rc4) < 0)
						throw std::runtime_error(SDL_GetError());
				}
				ApplySurface(xpos,
					ypos,
					gems[cell.color], ren, cell.size);
			}
		}
		int r1, c1, r0, c0;
		b.Hint(r0, c0, r1, c1);
		SDL_RenderDrawLine(ren, x + w * c0 + w / 2, y + h * r0 + h / 2, x + w * c1 + w / 2, y + h * r1 + h / 2);
		SDL_Rect point = {x + w * c0 + w / 4, y + h * r0 + h / 4, w / 2, h / 2};
		SDL_RenderFillRect(ren, &point);
		point.x = x + w * c1 + w / 4;
		point.y = y + h * r1 + h / 4;
		SDL_RenderFillRect(ren, &point);
		SDL_RenderSetClipRect(ren, NULL);

	}
	void DrawTime(int x, int y, Uint32 time, SDL_Renderer* ren)
	{
		std::ostringstream s;
		s << "Time left " << time << " seconds!";
		DrawText(x, y, s.str().c_str(), ren);
	}
	void DrawProgress(int x, int y, Uint32 time, SDL_Renderer* ren)
	{
      SDL_Rect rc = {x, y, 270, 40};
      const int margin = 5;
      DrawFrame(rc, ren, margin);
      rc.x += margin;
      rc.y += margin;
      rc.w -= 2 * margin;
      rc.h -= 2 * margin;
      rc.w = rc.w * time / 60 / 1000;
      SDL_SetRenderDrawColor(ren, 0x00, 0x35, 0x00, 0xff);
      SDL_RenderFillRect(ren, &rc);
	}
   void DrawText(int x, int y, const char* text, SDL_Renderer* ren)
   {
      Guards::Texture tScore = make_font_texture(text, ren);
      int text_w, text_h;
      SDL_QueryTexture(tScore.obj_, NULL, NULL, &text_w, &text_h);
      const int margin = 10;
      SDL_Rect pos = {x, y, (margin * 4 + text_w), (margin + text_h) * 2};
      DrawFrame(pos, ren, margin);
      pos.x += + (pos.w - text_w) / 2;
      pos.y +=  + (pos.h - text_h) / 2;
      ApplySurface(pos.x, pos.y, tScore.obj_, ren);
   }
   void DrawFrame(const SDL_Rect& rc, SDL_Renderer* ren, int margin)
   {
      SDL_Rect pos = rc;
      SDL_SetRenderDrawColor(ren, 0x30, 0x90, 0xf0, 0x80);
      SDL_RenderFillRect(ren, &pos);
      pos.w -= margin * 2;
      pos.h -= margin * 2;
      pos.x += margin;
      pos.y += margin;
      SDL_SetRenderDrawColor(ren, 0x90, 0x90, 0x90, 0x80);
      SDL_RenderFillRect(ren, &pos);
   }
   void DrawFinalScore(int x, int y, int score, SDL_Renderer* ren)
   {
         std::ostringstream s;
         s << "Time's up! Final score " << score << "!";
         DrawText(x, y, s.str().c_str(), ren);
   }
	int GemWidth() const {return gem_width;}
	int GemHeight() const {return gem_height;}
	~GameDrawer()
	{
		for(Textures::size_type i = 0; i < help.size(); i++)
			SDL_DestroyTexture(help[i]);
		for(Textures::size_type i = 0; i < gems.size(); i++)
			SDL_DestroyTexture(gems[i]);
		SDL_DestroyTexture(background);
	}
private:
	GameDrawer(const GameDrawer&);
	GameDrawer& operator=(const GameDrawer&);

	SDL_Texture* make_font_texture(const char* text, SDL_Renderer* ren)
	{
		SDL_Color clrFg = {0xf0,0xf0,0xf0};

		Guards::Surface sText = TTF_RenderText_Solid(main_font.obj_, text, clrFg );
		if(!sText.obj_)
			throw std::runtime_error(SDL_GetError());

		SDL_Texture* tex_font = SDL_CreateTextureFromSurface(ren, sText.obj_);
		if(!tex_font)
			throw std::runtime_error(SDL_GetError());

		return tex_font;
	}

	Textures help;
	Textures gems;
	SDL_Texture* background;
	int gem_width;
	int gem_height;
	Guards::Font main_font;
};

int game_main()
{
 //The images 
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1){
		std::cerr << SDL_GetError() << std::endl;
		return 1;
	}
	TTF_Init();

   Conf conf;
   Guards::Surface bmp = IMG_Load(conf.GetBackground().c_str());
	if (!bmp.obj_)
	{
		std::cerr << SDL_GetError() << std::endl;
		return 1;
	}	

	SDL_Window *win = SDL_CreateWindow("GEMS!", 100, 100, bmp.obj_->w, bmp.obj_->h, SDL_WINDOW_SHOWN);
	if (win == 0){
		std::cerr << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, 
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == 0)
	{
		std::cerr << SDL_GetError() << std::endl;
		return 1;
	}	

	try {
		GameDrawer drawer(ren, bmp.obj_, conf.GetGems(), conf.GetFontFile());
		bmp.Free();

		bool quit = false;
		SDL_Event e; 

		const int boardx = 340;
		const int boardy = 120;

		BoardController ctrl(8, 8, conf.GetGems().size());
		Uint32 time;
		const Uint32 game_time = 1000 * 60;
		bool has_played = false;
		while (!quit){
			while(SDL_PollEvent(&e)){
				//If user closes he window
				if (e.type == SDL_QUIT)
					quit = true;
				//If user presses any key
				if (e.type == SDL_KEYDOWN)
				{
					switch(e.key.keysym.sym)
					{
					case SDLK_ESCAPE:
						quit = true;
						break;
					case SDLK_SPACE:
						ctrl.Start();
						time = SDL_GetTicks();
						break;
					}
				}
				//If user clicks the mouse
				if (e.type == SDL_MOUSEBUTTONUP && ctrl.IsRunning())
				{
					const int c = (e.button.x - boardx) / drawer.GemWidth();
					const int r = (e.button.y - boardy) / drawer.GemHeight();
					if (!ctrl.Selected()) 
					{
						ctrl.Select(r, c);
					}
					else 
					{
						ctrl.Swap(r, c);
					}
				}
			}
			SDL_RenderClear(ren);

			drawer.DrawBackground(ren);
			drawer.DrawHelp(10, 10, ren);
			drawer.RenderBoard(boardx, boardy, ctrl.GetBoard(), 
				ctrl.SelRow(), ctrl.SelCol(), ren);
			if(ctrl.IsRunning())
			{
				ctrl.Update();
				Uint32 ellapsedTime = SDL_GetTicks() - time;
				if(ellapsedTime > game_time)
				{
					ctrl.Stop();
					has_played = true;
				}
				else
				{
					drawer.DrawTime(20, 200, (game_time - ellapsedTime) / 1000, ren);
					drawer.DrawProgress(boardx, 440, game_time - ellapsedTime, ren);
					std::ostringstream s;
					s << "Your score " << ctrl.GetBoard().TotalBurned() << "!";
					drawer.DrawText(20, 300, s.str().c_str(), ren);
				}
			}
			else if(has_played)
			{
				drawer.DrawFinalScore(boardx, boardy, ctrl.GetBoard().TotalBurned(), ren);
			}

			SDL_RenderPresent(ren);
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << "An expetion occured" << std::endl;
		std::cerr << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "Unknown exception " << std::endl;
	}

	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);

	TTF_Quit();
	SDL_Quit();
	return 0;
}
