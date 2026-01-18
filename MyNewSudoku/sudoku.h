#pragma once

#ifdef SUDOKU_EXPORTS
#define SUDOKU_API __declspec(dllexport)
#else
#define SUDOKU_API __declspec(dllimport)
#endif

#include <random>

constexpr int MAX_ROWS{ 9 };
constexpr int MAX_COLS{ 9 };
constexpr int CLEAR_VALUE{ -1 };

constexpr float scr_width{ 460.0f };
constexpr float scr_height{ 560.0f };

constexpr float sky{ 50.0f };
constexpr float ground{ 510.0f };

struct SUDOKU_API FPOINT
{
	float x{ 0 };
	float y{ 0 };
};
struct SUDOKU_API FRECT
{
	float left{ 0 };
	float up{ 0 };
	float right{ 0 };
	float down{ 0 };
};

namespace dll
{
	class SUDOKU_API RANDIT
	{
	private:
		std::mt19937* twister{ nullptr };

	public:
		RANDIT();

		~RANDIT();

		int operator()(int min, int max);

		float operator()(float min, float max);
	};
	
	class SUDOKU_API PROTON
	{
	private:
		float _width{ 0 };
		float _height{ 0 };

		bool _in_heap = false;

	public:
		FPOINT start{};
		FPOINT end{};
		FPOINT center{};

		PROTON();
		PROTON(float _sx, float _sy);
		PROTON(float _sx, float _sy, float _s_width, float _s_height);

		virtual ~PROTON() {};

		void Release();

		void set_edges();
		void new_dims(float new_width, float new_height);
		void new_width(float new_width);
		void new_height(float new_height);

		float get_width()const;
		float get_height()const;

		bool in_heap()const;

		static PROTON* create(float sx, float sy, float s_width, float s_height);
	};

	struct SUDOKU_API TILE
	{
		PROTON dims{};
		int quadrant = -1;
		int value = CLEAR_VALUE;
		int row = -1;
		int col = -1;
		bool valid_number = true;
	};

	class SUDOKU_API GRID
	{
	private:
		TILE game_grid[MAX_ROWS][MAX_COLS]{};

	public:
		GRID();

		int get_value(int row, int col)const;
		int get_quadrant(int row, int col)const;

		void set_value(int row, int col, int new_value);
		bool value_ok(int row, int col)const;
		void clear_grid();
		void set_level(int level);

		FRECT get_dims(int row, int col) const;
	};
}