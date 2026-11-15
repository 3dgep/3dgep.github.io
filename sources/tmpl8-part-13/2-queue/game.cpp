#include "game.hpp"

#include <cassert>
#include <iostream>

namespace Tmpl8
{
    constexpr int CellSize = 32;                    // the board is made of cells of 32 x 32 pixels
    constexpr int BoardWidth = 25, BoardHeight = 16; // 25 x 16 cells: the whole screen
    constexpr float StepTime = 100;                 // the snake moves one cell every 100 milliseconds

    Sprite tiles("assets/snake.png", 8, 2);

    struct Cell
    {
        int x, y;
    };

    // A queue of cells: cells are added at the back, and taken off at the front. Inside, it's a ring buffer: the
    // front can be anywhere in the array, and the cells wrap around at the end of the array.
    class CellQueue
    {
    public:
        // Adds a cell at the back. Returns false (and adds nothing) when the queue is full.
        bool PushBack(Cell cell)
        {
            if (count == Capacity) return false;
            cells[(front + count) % Capacity] = cell;
            count++;
            return true;
        }
        // Takes the cell at the front off the queue (when the queue is empty, nothing happens).
        void PopFront()
        {
            if (count == 0) return;
            front = (front + 1) % Capacity;
            count--;
        }
        // The cell with index i: 0 is the front. Only for the cells in the queue (i from 0 to Size() - 1).
        Cell At(int i) const
        {
            assert(i >= 0 && i < count);
            return cells[(front + i) % Capacity];
        }
        Cell Back() const { return At(count - 1); }
        int Size() const { return count; }
        bool Empty() const { return count == 0; }
        void Clear() { front = count = 0; }
    private:
        static constexpr int Capacity = BoardWidth * BoardHeight; // never more cells than the board has
        Cell cells[Capacity];
        int front = 0; // the index of the first cell in the array
        int count = 0; // the number of cells in the queue
    };

    CellQueue snake;   // the front is the tail of the snake, the back is its head
    Cell apple;
    int heading;       // 0 is up, 1 is right, 2 is down, 3 is left
    int lastStep;      // the heading of the last step
    Cell directions[4] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
    bool alive;
    float timer;

    // Is the cell part of the snake? (Checks every cell of the snake.)
    bool OnSnake(Cell cell)
    {
        for (int i = 0; i < snake.Size(); i++)
            if (snake.At(i).x == cell.x && snake.At(i).y == cell.y) return true;
        return false;
    }

    void PlaceApple()
    {
        do
        {
            apple = { 1 + IRand(BoardWidth - 2), 1 + IRand(BoardHeight - 2) }; // inside the walls
        } while (OnSnake(apple));
    }

    void Start()
    {
        snake.Clear();
        for (int x = 5; x <= 12; x++)
            snake.PushBack({ x, 8 });
        heading = lastStep = 1;
        alive = true;
        timer = 0;
        PlaceApple();
    }

    // Moves the snake one cell forward.
    void Step()
    {
        lastStep = heading;
        const Cell head = snake.Back();
        const Cell next = { head.x + directions[heading].x, head.y + directions[heading].y };
        const Cell tail = snake.At(0);
        const bool eats = next.x == apple.x && next.y == apple.y;
        // a wall, or the snake itself (but not the tail, when it moves away in this step)
        const bool wall = next.x == 0 || next.y == 0 || next.x == BoardWidth - 1 || next.y == BoardHeight - 1;
        const bool tailLeaves = !eats && next.x == tail.x && next.y == tail.y;
        if (wall || (OnSnake(next) && !tailLeaves))
        {
            alive = false;
            return;
        }
        if (!eats) snake.PopFront();
        snake.PushBack(next);
        if (eats) PlaceApple();
    }

    void DrawCell(Surface& target, int frame, Cell cell)
    {
        tiles.SetFrame(frame);
        tiles.Draw(target, cell.x * CellSize, cell.y * CellSize);
    }

    // -----------------------------------------------------------
    // Initialize the application
    // -----------------------------------------------------------
    void Game::Init()
    {
        Start();
    }

    // -----------------------------------------------------------
    // Close down application
    // -----------------------------------------------------------
    void Game::Shutdown()
    {
    }

    // -----------------------------------------------------------
    // Main application tick function
    // -----------------------------------------------------------
    void Game::Tick(float deltaTime)
    {
        if (alive)
        {
            // the keys choose a direction; the snake can only turn to the side (not turn around)
            int wanted = heading;
            if (GetKeyPressed(SDL_SCANCODE_UP) || GetKeyPressed(SDL_SCANCODE_W)) wanted = 0;
            if (GetKeyPressed(SDL_SCANCODE_RIGHT) || GetKeyPressed(SDL_SCANCODE_D)) wanted = 1;
            if (GetKeyPressed(SDL_SCANCODE_DOWN) || GetKeyPressed(SDL_SCANCODE_S)) wanted = 2;
            if (GetKeyPressed(SDL_SCANCODE_LEFT) || GetKeyPressed(SDL_SCANCODE_A)) wanted = 3;
            if (wanted % 2 != lastStep % 2) heading = wanted;
            timer += deltaTime;
            if (timer >= StepTime)
            {
                timer -= StepTime;
                Step();
            }
        }
        else if (GetKeyPressed(SDL_SCANCODE_SPACE)) Start();

        // the board: walls around the edge, grass (with a few flowers) inside
        for (int y = 0; y < BoardHeight; y++)
            for (int x = 0; x < BoardWidth; x++)
            {
                const bool wall = x == 0 || y == 0 || x == BoardWidth - 1 || y == BoardHeight - 1;
                DrawCell(screen, wall ? 2 : (x * 7 + y * 13) % 9 == 0 ? 1 : 0, { x, y });
            }
        DrawCell(screen, 3, apple);
        for (int i = 0; i < snake.Size() - 1; i++)
            DrawCell(screen, 4 + i % 2, snake.At(i));
        DrawCell(screen, (alive ? 8 : 12) + heading, snake.Back());
    }
}
