// http://stackoverflow.com/questions/5739611/differences-between-abstract-factory-pattern-and-factory-method


#include <cstdio>
using namespace std;

class Maze {
public:
	Maze() {
		printf("create a maze\n");
	}
};

class MazeFactory {
public:
	static MazeFactory* Instance() { // singleton pattern
		if (_Instance == 0) {
			_Instance = new MazeFactory;
		}
		return _Instance;
	}

	virtual Maze* MakeMaze() const {
		return new Maze;
	}

	MazeFactory() { // constructor is "public"for abstract factory. If singleton, this should be "protected"
		printf("MazeFactory::MazeFactory()\n"); 
	}

private:
	static MazeFactory* _Instance;	

};

class BombedMazeFactory: public MazeFactory {
public:
	BombedMazeFactory() {
		printf("BombedMazeFactory::BombedMazeFactory()\n");
	}
};

class MazeGame {
public:
	Maze* CreateMaze() {
		
		printf("MazeGame::CreateMaze()\n");
	    Maze* aMaze = MakeMaze();
    	return aMaze;
	}

	Maze* CreateMaze(MazeFactory& factory) {
		Maze* amaze = factory.MakeMaze();
		return amaze;
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
	MazeGame game;
	BombedMazeFactory factoryBombed;

	game.CreateMaze(factoryBombed);

	return 0;
}