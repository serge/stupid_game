#pragma once

#include "board.h"

class BoardController
{
public:
	BoardController(int r, int col, int colors);
    void Start();
	bool IsRunning() const;
	void Stop();
	bool Selected() const;
	void Select(int r, int c);
	int SelRow() const;
	int SelCol() const;
	void Swap(int r, int c);
	const Board& GetBoard() const;
	void Update();
private:
	void unselect();
	int r_;
	int c_;
	bool started_;
	Board board_;
};
