#include "stdafx.h"
#include "BoardCtrl.h"

BoardController::BoardController(int r, int c, int colors)
:board_(r, c, colors)
{
	unselect();
	Stop();
}

void BoardController::unselect()
{
	r_ = c_ = -1;
}

void BoardController::Start()
{
	unselect();
	started_ = true;
   board_.Reinit();
}

bool BoardController::IsRunning() const
{
	return started_;
}

void BoardController::Stop()
{
	started_ = false;
}

bool BoardController::Selected() const
{
	return r_ != -1 && c_ != -1;
}

void BoardController::Select(int r, int c)
{
	r_ = r;
	c_ = c;
}

void BoardController::Swap(int r, int c)
{
	board_.Swap(r_, c_, r, c);
	unselect();
}

const Board& BoardController::GetBoard() const
{
	return board_;
}

void BoardController::Update()
{
	board_.Update();
}

int BoardController::SelRow() const
{
	return r_;
}

int BoardController::SelCol() const
{
	return c_;
}

