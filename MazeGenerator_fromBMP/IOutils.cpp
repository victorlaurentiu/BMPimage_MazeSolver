#include "IOutils.h"

Maze *createMatFromBMPFile(vector<unsigned char> & imagedata) {
	unsigned char bpp, padding;

	uint16_t offset = imagedata[11];
	offset <<= 8;
	offset += imagedata[10];

	uint16_t width = imagedata[19];
	width <<= 8;
	width += imagedata[18];

	uint16_t height = imagedata[23];
	height <<= 8;
	height += imagedata[22];

	bpp = imagedata[28] / 8;	// bytes per pixel (bits/8 = bytes)
	padding = (4 - (width * bpp) % 4) % 4;

	/* filling the maze */
	Maze *maze = new Maze(height, width);
	system("cls");
	maze->printMaze();
	int k = offset;
	for (int i = height - 1; i >= 0; i--) {
		for (int j = 0; j < width; j++) {
			int s = 0;
			for (int b = 0; b < bpp; b++) {
				s += (int)imagedata[k + b];
			}
			if (s < 215 * bpp) { // is pixel almost black
				maze->setVal(i, j, Maze::BLOCK);
			}
			k += bpp;
		}
		k += padding;
	}

	return maze;
}

/**
 * maze - the matrix containing 0's and 1's
 * sol - array of positions representing the solution path
 * path - array of directions for each position
 * imgdata - image data used to write another image using the old one
*/
void writeBMPFileFromMat(Maze *maze, FILE* fout,
	vector<path_cell> &path,	// position, direction
	vector<unsigned char> & imgdata)
{
	unsigned char bpp, padding;

	uint16_t offset = imgdata[11];
	offset <<= 8;
	offset += imgdata[10];

	int width = maze->getNrCols();
	int height = maze->getNrRows();
	bpp = imgdata[28] / 8;	// bytes per pixel (bits/8 = bytes)
	padding = (4 - (width * bpp) % 4) % 4;

	// marking the path with '-2' to colour it differently
	vec2 v;
	int path_size = path.size();
	for (int i = 1; i < path_size; i++) {
		// mark neigbours to thicken the line
		for (vec2 dir : directions) {
			v = path[i].position + dir;
			if (maze->getVal(v) != Maze::BLOCK)
				maze->setVal(v, PATH_ENCODING);
		}

		// mark current position
		maze->setVal(path[i].position, PATH_ENCODING);
	}
	maze->setVal(path[path_size - 1].position, PATH_ENCODING);

	// rewriting imgdata with the path coloured
	int k = offset;
	for (int i = height-1; i >= 0; i--) {
		for (int j = 0; j < width; j++) {
			int s = 0;
			for (int b = 0; b < bpp; b++)
				if (maze->getVal(i, j) == PATH_ENCODING)
					imgdata[k + b] /= 2;	// changing white to half white (i.e. gray)
			k += bpp;
		}
		k += padding;
	}

	fwrite(imgdata.data(), 1, imgdata.size(), fout);
}

// prints the maze with all the distances (used for debugging in bfs)
void printMat(const Maze & maze) {
	int n = maze.getNrRows();
	int m = maze.getNrCols();
	int block = 219;

	for (int i = 0; i < n; i++)	{
		for (int j = 0; j < m; j++) {
			if (maze.getVal(i, j) == Maze::BLOCK)
				printf("%c%c%c%c", block, block, block, block);
			else
				printf("%-3d ", maze.getVal(i, j));
		}
		cout << "\n";
	}
}

void readWhatToCrop(const Maze & maze) {
	while (true) {
		cout << "Show specific crop of the maze?\n y/n\n";
		if (_getch() == 'y') {
			vec2 center;
			vec2 r;
			cout << "Write center and radius (Format: cx cy rx ry)\n";
			cin >> center.x >> center.y >> r.x >> r.y;
			maze.printMazeCrop(center, r);
		}
		else {
			break;
		}
	}
}

static bool isInBounds(const Maze &maze, const vec2 &V) {
	return V.x >= 0 && V.x < maze.getNrRows()
		&& V.y >= 0 && V.y < maze.getNrCols();
}

void readStartPos(const Maze & maze, vec2 & Start) {
	cout << "For Start point ";
	readWhatToCrop(maze);

readStartPosLabel:
	cout << "Write the coord for start position (Format: x y)\n";

	cin >> Start.x >> Start.y;

	if (!isInBounds(maze, Start) || maze.getVal(Start) == Maze::BLOCK)
	{
		cout << "Outside Maze or Wall position => Try again\n";
		goto readStartPosLabel;	// try again
	}
}

vector<vec2> readEndPos(const Maze & maze) {
	int height = maze.getNrRows();
	int width = maze.getNrCols();

	int nr_exits;
	cout << "How many exits?\n";
	cin >> nr_exits;

	vec2 exit;	// End coordonates
	vector<vec2> exits;
	for (int i = 0; i < nr_exits; i++) {
		while (true) {
			readWhatToCrop(maze);

			cout << "Write the coordinates for exit position (Format: x y)\n";
			cin >> exit.x >> exit.y;

			if (!isInBounds(maze, exit) || maze.getVal(exit) == Maze::BLOCK)
			{
				cout << "Outside Maze or Wall position => Try again\n";
				continue;
			}

			break;
		}

		exits.push_back(exit);
	}

	return exits;
}
