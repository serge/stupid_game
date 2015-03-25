#include "stdafx.h"
#include "board.h"
#include <cmath> 

class NormalRowCol {
public:
	static int GetRow(int r, int) {return r;}
	static int GetCol(int, int c) {return c;}
};

class InversedRowCol {
public:
	static int GetRow(int, int c) {return c;}
	static int GetCol(int r, int) {return r;}
};

Board::Board(int row, int col, int colors)
  :col_(col), 
  row_(row), 
  full_height_(row * 2 + 1), 
  board_(full_height_ * col),
  min_burn_len_(3),
  color_num_(colors),
  total_burned_(0)
{
   Reinit();
}

void Board::Reinit()
{  
   total_burned_ = 0;
	for(size_t i = 0; i < board_.size(); i++)
	{
		const Type type = {GS_HIDDEN, rand() % color_num_, 1, 0};
		board_[i] = type;
	}
	for(int r = 0; r < row_; r++) 
		for(int c = 0; c < col_; c++) 
		{
			get_type(r, c).state = GS_NONE;
		}
	for(int c = 0; c < col_; c++) 
	{
		board_[(full_height_ - 1) * col_ + c].state = GS_BORDER;
	}
	while(true)
	{
		const Points pts = check_area(0, 0, full_height_ - 1, col_);
		if(pts.empty())
			break;
		for(Points::size_type i = 0; i < pts.size(); i++)
		{
			Type& cell = get_type_full_index(pts[i].r, pts[i].c);
			cell.color = pick_color(pts[i].r, pts[i].c);
		}
	}
}

inline int Board::pick_color(int r, int c) const
{
	int colorl[] = { r + 1 < full_height_ - 1 ? 
		get_type_full_index( r + 1,  c).color: color_num_,
		 r - 1 >= 0 ? get_type_full_index( r - 1,  c).color: color_num_,
		 c - 1 >= 0 ? get_type_full_index( r,  c - 1).color: color_num_,
		 c + 1 < col_ ? get_type_full_index( r,  c + 1).color: color_num_};
	int color = 0;
	bool used = true;
	while(used)
	{
		used = false;
		for(int i = 0; i < sizeof(colorl) / sizeof(*colorl); i++)
			if(color == colorl[i])
			{
				used = true;
				color++;
				break;
			}
		if(color >= color_num_)
			throw std::runtime_error("Not enough color to choose from");
	}
	return color;
}

const Type& Board::Get(int r, int c) const
{
	return get_type(r, c);
}

inline Type& Board::get_type_full_index(int r, int c)
{
	return const_cast<Type&>(const_cast<const Board*>(this)->get_type_full_index(r, c));
}

inline const Type& Board::get_type_full_index(int r, int c) const
{
	return board_[get_full_index(r, c)];
}

Type& Board::get_type(int r, int c)
{
	const Type& cell = const_cast<const Board*>(this)->get_type(r, c);
	return const_cast<Type&>(cell);
}

const Type& Board::get_type(int r, int c) const
{
	return board_[get_index(r, c)];
}

int inline Board::get_index(int r, int c) const
{
	return (row_ + r) * col_ + c;
}

int inline Board::get_full_index(int r, int c) const
{
	return r * col_ + c;
}

template<class MetaDir>
void Board::initiate_swap(int row1, int col1, int row2, int col2, GEM_STATE state)
{
	if(MetaDir::GetCol(row1, col1) == MetaDir::GetCol(row2, col2)
		&& abs(MetaDir::GetRow(row1, col1) - MetaDir::GetRow(row2, col2)) == 1) {
		int upperRow = MetaDir::GetRow(row2, col2);
		int lowerRow = MetaDir::GetRow(row1, col1);
		int commonCol = MetaDir::GetCol(row1, col1);
		if(upperRow < lowerRow) 
			std::swap(upperRow, lowerRow);
		Type& upper = get_type(MetaDir::GetRow(upperRow, commonCol), MetaDir::GetCol(upperRow, commonCol));
		Type& lower = get_type(MetaDir::GetRow(lowerRow, commonCol), MetaDir::GetCol(lowerRow, commonCol));
		upper.offset = -0.05;
		lower.offset = 0.05;
		upper.state = lower.state = state;
	}
}

void Board::Swap(int row1, int col1, int row2, int col2)
{
	initiate_swap<NormalRowCol>(row1, col1, row2, col2, GS_SWAP_VER);
	initiate_swap<InversedRowCol>(row1, col1, row2, col2, GS_SWAP_HOR);
}

void Board::burn()
{
	const Points p = check_area(0, 0, full_height_, col_);
	for(Points::size_type i = 0; i < p.size(); i++)
		get_type_full_index(p[i].r, p[i].c).state = GS_BURNING;
}

Board::Points Board::check_area(int r0, int c0, int r1, int c1) const
{
	Points pts;
    find_cell_to_burn<NormalRowCol>(r0, c0, r1, c1, pts);
    find_cell_to_burn<InversedRowCol>(r0, c0, r1, c1, pts);
	return pts;
}

template<class MetaDir>
void Board::find_cell_to_burn(int ur0, int uc0, int ur1, int uc1, Points& pts) const
{
	const int r0 = std::max(ur0, 0);
	const int c0 = std::max(uc0, 0);
	const int r1 = std::min(ur1, full_height_ - 1);
	const int c1 = std::min(uc1, col_);
	for(int r = MetaDir::GetRow(r0, c0); r < MetaDir::GetRow(r1, c1); r++)
	{
		int color = get_type_full_index(MetaDir::GetRow(r, c0), MetaDir::GetCol(r, c0)).color;
		int start_c = MetaDir::GetCol(r0, c0);
		for(int c = MetaDir::GetCol(r0, c0); c < MetaDir::GetCol(r1, c1); c++)
		{
			const int next_color = get_type_full_index(MetaDir::GetRow(r, c), MetaDir::GetCol(r, c)).color;
			if(color != next_color)
			{
				mark_as_empty<MetaDir>(start_c, c, r, pts);
				start_c = c;
				color = next_color;
			}
		}
		mark_as_empty<MetaDir>(start_c, MetaDir::GetCol(r1, c1), r, pts);
	}
}

template<class MetaDir>
void Board::mark_as_empty(int start_c, int c, int r, Points& pts) const
{
	if(c - start_c >= min_burn_len_)
		for(;start_c < c; start_c++) 
		{
			const int mr = MetaDir::GetRow(r, start_c);
			const int mc = MetaDir::GetCol(r, start_c);
			const Pos pos = {mr, mc};
			pts.push_back(pos);
		}
}

int Board::Col() const
{
   return col_;
}

int Board::Row() const
{
   return row_;
}

template<class MetaDir>
bool Board::check_local_area(int r0, int c0, int r1, int c1) const
{
	const int minRow = std::min(r0, r1) - min_burn_len_;
	const int maxRow = std::max(r0, r1) + min_burn_len_;
	const int minCol = std::min(c0, c1) - min_burn_len_;
	const int maxCol = std::max(c0, c1) + min_burn_len_;
	return !check_area(
		MetaDir::GetRow(minRow, minCol), 
		MetaDir::GetCol(minRow, minCol), 
		MetaDir::GetRow(maxRow, maxCol), 
		MetaDir::GetCol(maxRow, maxCol)
		).empty();
}

template<class MetaDir>
bool Board::on_swap_t(int r, int c, GEM_STATE swap_state, GEM_STATE back_state)
{
	Type& cell = get_type_full_index(r, c);
	const int destCol = MetaDir::GetCol(
		static_cast<int>(floor(r + cell.offset)),
		static_cast<int>(floor(c + cell.offset)));
	const int destRow = MetaDir::GetRow(r, c);
	Type& sibling = get_type_full_index(
		MetaDir::GetRow(destRow, destCol), 
		MetaDir::GetCol(destRow, destCol));
	bool need_burn = false;
	if(abs(cell.offset) >= 1)
	{
		if(sibling.state == swap_state)
		{
			std::swap(cell, sibling);
			cell.state = sibling.state = GS_NONE;
			if(check_local_area<MetaDir>(r, c, destRow, destCol))
				need_burn = true;
			else
			{
				cell.state = sibling.state = back_state;
				cell.offset = cell.offset > 0 ? -0.1: 0.1;
				sibling.offset = sibling.offset > 0 ? -0.1: 0.1;
			}
		}
		else if(sibling.state == back_state)
		{
			std::swap(cell, sibling);
			cell.state = sibling.state = GS_NONE;
		}
	}
	else
		cell.offset += cell.offset > 0 ? 0.1: -0.1;
	return need_burn;
}

void Board::Update() 
{
	int falling_cells = 0;
	bool need_burn = false;
	for(int r = 0; r < full_height_; r++)
	{
		for(int c = 0; c < col_; c++)
		{
			Type& cell = get_type_full_index(r, c);
			switch(cell.state)
			{
			case GS_BURNING:
				falling_cells++;
				if(cell.size <= 0)
				{
					cell.size = 1;
					cell.state = GS_BURNED;
					total_burned_++;
				}
				else
				{
					cell.size -= 0.1;
				}
				break;
			case GS_SWAP_BACK_HOR:
			case GS_SWAP_HOR:
				need_burn = on_swap_t<NormalRowCol>(r, c, GS_SWAP_HOR, GS_SWAP_BACK_HOR);
				break;
			case GS_SWAP_VER:
			case GS_SWAP_BACK_VER:
				need_burn = on_swap_t<InversedRowCol>(r, c, GS_SWAP_VER, GS_SWAP_BACK_VER);
				break;
			case GS_FALLING:
				falling_cells++;
				cell.offset += 0.1;
				if(cell.offset > 1)
				{
					const int curRow = r + static_cast<int>(cell.offset) + 1;
					Type& overlaped = get_type_full_index(curRow, c);
					if(overlaped.state == GS_NONE || overlaped.state == GS_BORDER)
					{
						cell.state = GS_NONE;
						falling_cells--;
						cell.offset = 0;
						std::swap(cell, get_type_full_index(curRow - 1, c));
					}
				}
				break;
			}
		}
	}
	if(!falling_cells || need_burn)
	{
		replenish_cells();
		burn();
	}
	collect_burned_cells();
}

void Board::collect_burned_cells()
{
	for(int c = 0; c < col_; c++)
	{
		int burned = 0;
		for(int r = row_ - 1; r >= 0; r--)
		{
			Type& cell = get_type(r, c);
			if(cell.state == GS_BURNED)
			{
				cell.state = GS_EMPTY;
				burned++;
			}
			if(cell.state == GS_NONE && burned > 0)
			{
				cell.state = GS_FALLING;
				cell.offset = 0;
			}
		}
		int hr = - 1;
		for(;burned > 0;burned--)
		{
			while(get_type(hr, c).state != GS_HIDDEN)
				hr--;
			Type& cell = get_type(hr, c);
			cell.state = GS_FALLING;
			cell.offset = 0;
		}
	}
}

void Board::replenish_cells()
{
	for(int r = 0; r < row_; r++)
		for(int c = 0; c < col_; c++)
		{
			Type& cell = get_type_full_index(r, c);
			if(cell.state != GS_HIDDEN)
			{
				cell.color = pick_color(r, c);
				cell.state = GS_HIDDEN;
			}
		}
}

int Board::TotalBurned() const
{
	return total_burned_;
}

void Board::Hint(int& r0, int& c0, int& r1, int& c1) const
{
	for(int r = row_ + min_burn_len_; r < full_height_ - 2; r++)
	{
		for(int c = 0; c < col_ - 1; c++)
		{
			Type& cell = const_cast<Type&>(get_type_full_index(r, c));
			Type& left_cell = const_cast<Type&>(get_type_full_index(r, c + 1));
			std::swap(cell, left_cell);
			const bool isHint = check_local_area<NormalRowCol>(r, c, r, c + 1);
			std::swap(cell, left_cell);
			if(isHint)
			{
				r0 = r - row_;
				c0 = c;
				r1 = r - row_;
				c1 = c + 1;
				return;
			}
		}
	}
}