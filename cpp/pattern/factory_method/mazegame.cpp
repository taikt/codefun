#include <cstdio>
using namespace std;

class Maze {
public:
	Maze() {
		printf("create a maze\n");
	}
};

class MazeGame {
public:
	Maze* CreateMaze() {
		
		printf("MazeGame::CreateMaze()\n");
	    Maze* aMaze = MakeMaze();
    	return aMaze;
	}

	// factory method
	virtual Maze* MakeMaze() const
    { return new Maze; }
};

class BombedMazeGame: public MazeGame {
public:
	BombedMazeGame() {
		printf("BombedMazeGame::BombedMazeGame()\n");
	}

	//override this method, it is a factory method
	Maze* CreateMaze() {
		printf("BombedMazeGame MazeGame::CreateMaze()\n");
	    Maze* aMaze = MakeMaze();
    	return aMaze;
	}
};

int main() {

	MazeGame myGame;
    Maze* m1 = myGame.CreateMaze();
    
    BombedMazeGame myBombedGame;
    Maze* m2 = myBombedGame.CreateMaze();

	return 0;
}