#pragma once

#include <vector>

typedef enum {
GS_NONE,
GS_BURNING,
GS_BURNED,
GS_EMPTY,
GS_SWAP_HOR,
GS_SWAP_VER,
GS_SWAP_BACK_VER,
GS_SWAP_BACK_HOR,
GS_FALLING,
GS_BORDER,
GS_HIDDEN,
} GEM_STATE;


typedef struct {
	GEM_STATE state;
	int color;
	double size;
	double offset;
} Type;

typedef struct 
{
	int r, c;
} Pos;

class Board
{
public:
   Board(int row, int col, int colors);
   const Type& Get(int row, int col) const;
   void Swap(int row1, int col1, int row2, int col2);
   int Row() const;
   int Col() const;
   void Update();
   int TotalBurned() const;
   void Reinit();
   void Hint(int& r0, int& c0, int& r1, int& c1) const;
private:
   typedef std::vector<Pos> Points;
   Points check_area(int r0, int c0, int r1, int c1) const;
   Type& get_type(int r, int c);
   const Type& get_type(int r, int c) const;
   Type& get_type_full_index(int r, int c);
   const Type& get_type_full_index(int r, int c) const;
   int inline get_index(int r, int c) const;
   int inline get_full_index(int r, int c) const;
   int inline pick_color(int r, int c) const;
   void replenish_cells();
   void burn();
   bool on_swap(int r, int c);
   void collect_burned_cells();

   template<class MetaDir>
   void find_cell_to_burn(int r0, int c0, int r1, int c1, Points& pts) const;
   template<class MetaDir>
   void mark_as_empty(int start_c, int c, int r, Points& points) const;
   template<class MetaDir>
   void initiate_swap(int row1, int col1, int row2, int col2, GEM_STATE state);
   template<class MetaDir>
   bool on_swap_t(int r, int c, GEM_STATE, GEM_STATE);
   template<class MetaDir>
   bool check_local_area(int r0, int c0, int r1, int c1) const;
private:
   int row_;
   int col_;
   int full_height_;
   std::vector<Type> board_;
   const int min_burn_len_;
   const int color_num_;
   int total_burned_;
};