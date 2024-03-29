#ifndef MAZE_H
#define MAZE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"

#include <string>
#include <iostream>
#include <vector>
#include <ctime>
#define DEBUG_MAZE_GENERATE

using namespace std;

struct MazePoint {
	int m_x, m_y;		//中心点坐标
	bool m_isVisited;	//是否被访问过
	int m_state;
	//各种状态：通路、墙、入口、出口
	enum {
		E_State_Path, E_State_Wall, E_State_Entry, E_State_Dest, E_State_Max,
	};

	MazePoint(int x = 0, int y = 0) {
		m_x = x;
		m_y = y;
		m_isVisited = false;	//一开始都是false
		m_state = E_State_Wall;
	}
	//设置坐标
	void setCoord(int x, int y) {
		m_x = x;
		m_y = y;
	}
	//设置是否被访问过的状态
	void setVisited(bool v = true) {
		m_isVisited = v;
	}
	//赋值
	void operator = (const MazePoint & point) {
		m_x = point.m_x;
		m_y = point.m_y;
		m_isVisited = point.m_isVisited;
	}
};

class Maze
{
private:
		int Col, Row, RawCol, RawRow;

		MazePoint** MazeArr;	//迷宫
		MazePoint** RawArr;		//通路
		MazePoint EntryPoint;	//入口
		MazePoint ExitPoint;	//出口
		MazePoint Current;		//当前位置
		vector<MazePoint> MazeStack;
public:
	//上下左右4个方向，E_Dir_Max表示数组大小（4）
	enum {
		E_Dir_Up, E_Dir_Down, E_Dir_Left, E_Dir_Right, E_Dir_Max,
	};

	Maze() {
		Col = Row = 0;
		MazeArr = NULL;
		RawArr = NULL;
		srand((int)time(0));
		rand();
	}

	virtual ~Maze() {
		reset();
	}
	//迷宫初始化
	void Init(int row = 8, int column = 10) {
		RawCol = column;
		RawRow = row;
		//分配内存并初始化位置
		RawArr = new MazePoint*[RawRow];
		for (int i = 0; i < RawRow; i++) {
			RawArr[i] = new MazePoint[RawCol];
			for (int j = 0; j < RawCol; j++) {
				RawArr[i][j].setCoord(j, i);
			}
		}
		//
		Row = 2 * RawRow + 1;
		Col = 2 * RawCol + 1;
		MazeArr = new MazePoint*[Row];
		for (int i = 0; i < Row; i++) {
			MazeArr[i] = new MazePoint[Col];
			for (int j = 0; j < Col; j++) {
				MazeArr[i][j].setCoord(j, i);
			}
		}
	}
/*
 * 清除迷宫
 * 就是将两个动态二维数组申请的内存释放
*/
	void reset() {
		if (MazeArr != NULL) {
			for (int i = 0; i < Row; i++) {
				delete[] (MazeArr[i]);
				MazeArr[i] = NULL;
			}
			delete[] MazeArr;
			MazeArr = NULL;
		}
		if (RawArr != NULL) {
			for (int i = 0; i < RawRow; i++) {
				delete[](RawArr[i]);
				RawArr[i] = NULL;
			}
			delete[] RawArr;
			RawArr = NULL;
		}
	}

/*
 * 画出3D迷宫
 ---> x
 |
 y
 对应到三维空间y不变
 z
 |
 ---> x
 
 这里定义的每一个方块大小为1*1*1，所以每一次的位移step都是1.0

*/
	void DrawMaze(Shader & shader, unsigned int & cubeVAO) {
		glm::mat4 model;
		//画出3D迷宫
		float step = 1.0;
		for (int i = 0; i < Row; i++) {
			for (int j = 0; j < Col; j++) {
				//画墙
				if (MazeArr[i][j].m_state == MazePoint::E_State_Wall) {
					model = glm::mat4();
					model = glm::translate(model, glm::vec3(step * j, 0.0f, -step * i));
					shader.setMat4("model", model);
					glBindVertexArray(cubeVAO);
					glDrawArrays(GL_TRIANGLES, 0, 36);
					glBindVertexArray(0);
				}
			}
		}
	}

//绘制右上角2D地图,传入的参数是当前相机位置
	void DrawMap(float & cameraX, float & cameraZ) {
		Shader shader("shader/line_shader.vs", "shader/line_shader.fs");
		float map[] = {
			0.0, 0.0, 0.0,
			0.0, 0.0, 0.0
		};
		//line VAO
		unsigned int lineVAO, lineVBO;
		glGenVertexArrays(1, &lineVAO);
		glGenBuffers(1, &lineVBO);
		glBindVertexArray(lineVAO);
		glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(map), &map, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);	//顶点位置
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
		shader.use();
		glLineWidth(3.0);
		//从右下角开始
		float step = 0.03;
		float xStart = 1;
		float yStart = 1;
		
		//画2D地图，线设为白色
		shader.setVec4("color", glm::vec4(1.0, 1.0, 1.0, 1.0));
		for (int i = Row - 1; i >= 0; i--) {
			for (int j = Col - 1; j >= 0; j--) {
				//迷宫的第i行第j列,是否加入这个点
				if (MazeArr[i][j].m_state == MazePoint::E_State_Wall) {
					//将其与四周存在的点连接
					//右边的点
					if (j + 1 < Col && MazeArr[i][j + 1].m_state == MazePoint::E_State_Wall) {
						map[0] = xStart - (Col - j) * step;
						map[1] = yStart - (Row - i) * step;
						map[3] = xStart - (Col - j - 1) * step;
						map[4] = yStart - (Row - i) * step;
						glBufferData(GL_ARRAY_BUFFER, sizeof(map), &map, GL_STATIC_DRAW);
						glBindVertexArray(lineVAO);
						glDrawArrays(GL_LINES, 0, 2);
						glBindVertexArray(0);
					}
					//下边的点
					if (i + 1 < Row && MazeArr[i + 1][j].m_state == MazePoint::E_State_Wall) {
						map[0] = xStart - (Col - j) * step;
						map[1] = yStart - (Row - i) * step;
						map[3] = xStart - (Col - j) * step;
						map[4] = yStart - (Row - i - 1) * step;
						glBufferData(GL_ARRAY_BUFFER, sizeof(map), &map, GL_STATIC_DRAW);
						glBindVertexArray(lineVAO);
						glDrawArrays(GL_LINES, 0, 2);
						glBindVertexArray(0);
					}
				}
			}
		}
		//显示当前在迷宫当中的位置
		float cameraStartX = xStart - Col * step;
		float cameraStartY = yStart - Row * step;
		float cameraNowX = cameraStartX + cameraX * step;
		float cameraNowY = cameraStartY - cameraZ * step;
		float cameraNow[3] = {
			cameraNowX, cameraNowY, 0.0
		};
		//画点
		glBufferData(GL_ARRAY_BUFFER, sizeof(cameraNow), &cameraNow, GL_STATIC_DRAW);
		glPointSize(5.0);
		glBindVertexArray(lineVAO);
		glDrawArrays(GL_POINTS, 0, 1);
		glBindVertexArray(0);
		
	}

// 碰撞检测
	bool checkCollisions(float & cameraX, float & cameraY, float objectSize, float & wallX, float & wallY, float wallSize) {
		// Collision x-axis?
		bool collisionX = cameraX + objectSize/2 >= wallX &&
			wallX + wallSize/2 >= cameraX;
		// Collision y-axis?
		bool collisionY = cameraY + objectSize / 2 >= wallY &&
			wallY + wallSize / 2 >= cameraY;
		// Collision only if on both axes
		return collisionX && collisionY;
	}

// 碰撞处理
	void doCollisions(Camera & camera, float objectSize, Camera_Movement direction, float deltaTime) {

		float cameraX = camera.Position.x;
		float cameraZ = camera.Position.z;

		float step = 0.03;
		float xStart = 1;
		float yStart = 1;
		float wallSize = 0.05f;

		//显示当前在迷宫当中的位置
		float cameraStartX = xStart - Col * step;
		float cameraStartY = yStart - Row * step;
		float cameraNowX = cameraStartX + cameraX * step;
		float cameraNowY = cameraStartY - cameraZ * step;

		bool collision = false;

		for (int i = Row - 1; i >= 0 && !collision; i--) {
			for (int j = Col - 1; j >= 0 && !collision; j--) {
				if (MazeArr[i][j].m_state == MazePoint::E_State_Wall) {
					float wallX = xStart - (Col - j) * step;
					float wallY = yStart - (Row - i) * step;
					collision = checkCollisions(cameraNowX, cameraNowY, objectSize, wallX, wallY, wallSize);
				}
			}
		}
		if (collision) {
			if (direction == LEFT || direction == RIGHT) {    // Horizontal collision
				if (direction == LEFT) {
					camera.ProcessKeyboard(RIGHT, 2 * deltaTime);
				}
				else {
					camera.ProcessKeyboard(LEFT, 2 * deltaTime);
				}
			}
			else { // Vertical collision
				if (direction == FORWARD) {
					camera.ProcessKeyboard(BACKWARD, 2 * deltaTime);
				}
				else {
					camera.ProcessKeyboard(FORWARD, 2 * deltaTime);
				}
			}
		}
	}

//自动生成迷宫
	void autoGenerateMaze() {
		//y行x列,x为横轴，y为竖轴
		
		int now_x = 0, now_y = 0;
		int next_x, next_y;

		//设置入口(固定0，0入口)
		EntryPoint.setCoord(now_x + 1, now_y);
		EntryPoint.m_state = MazePoint::E_State_Entry;
		//收集一些可能的出口
		vector<MazePoint> PosibleExitPoint;
		
		//初始位置（0，0）标记为被访问过，并压入栈中
		RawArr[now_y][now_x].setVisited(true);
		MazeStack.push_back(RawArr[now_y][now_x]);
		//如果还存在点未被访问，继续循环
		while (isThereUnvisited()) {
			next_x = now_x;
			next_y = now_y;
			if (getNeighbor(next_x, next_y)) {	//这个点四周还有可以走动的点（next_x,next_y被改变为走动以后的坐标）
				//访问 & 压栈
				RawArr[next_y][next_x].setVisited(true);
				MazeStack.push_back(RawArr[next_y][next_x]);
				//mark it passable between (curx,cury) => (nextx,nexty)
				MazeArr[2 * now_y + 1][2 * now_x + 1].m_state = MazePoint::E_State_Path;
				MazeArr[2 * next_y + 1][2 * next_x + 1].m_state = MazePoint::E_State_Path;
				MazeArr[now_y + next_y + 1][now_x + next_x + 1].m_state = MazePoint::E_State_Path;
				
				now_x = next_x;
				now_y = next_y;
			}
			else if (MazeStack.size() > 1) {
				//判断是否在MazeArr边界，如果在边界，就是可能的出口之一
				if (now_x  == RawCol - 1) {
					PosibleExitPoint.push_back(MazePoint(2 * now_x + 2, 2 * now_y + 1));
				}
				if (now_x == 0) {
					PosibleExitPoint.push_back(MazePoint(2 * now_x, 2 * now_y + 1));
				}
				if (now_y == RawRow - 1) {
					PosibleExitPoint.push_back(MazePoint(2 * now_x + 1, 2 * now_y + 2));
				}
				if (now_y == 0) {
					PosibleExitPoint.push_back(MazePoint(2 * now_x + 1, 2 * now_y));
				}
				MazeStack.pop_back();
				now_x = MazeStack[MazeStack.size() - 1].m_x;
				now_y = MazeStack[MazeStack.size() - 1].m_y;
			}
			else if (MazeStack.size() == 1) {
				now_x = MazeStack[MazeStack.size() - 1].m_x;
				now_y = MazeStack[MazeStack.size() - 1].m_y;
				MazeStack.pop_back();
			}
		}
		//随机选定一个出口
		int sizeE = PosibleExitPoint.size();
		int randIndex = random(0, sizeE);
		while (PosibleExitPoint[randIndex].m_y == 0 || PosibleExitPoint[randIndex].m_x == 0) {
			randIndex = random(0, sizeE);
		}
		ExitPoint.setCoord(PosibleExitPoint[randIndex].m_x, PosibleExitPoint[randIndex].m_y);
		ExitPoint.m_state = MazePoint::E_State_Dest;

		//在MazeArr中表明入口&出口
		MazeArr[EntryPoint.m_y][EntryPoint.m_x].m_state = MazePoint::E_State_Entry;
		MazeArr[ExitPoint.m_y][ExitPoint.m_x].m_state = MazePoint::E_State_Dest;
	}
	
/*
 * 是否所有的点都已经被访问过了
 * 全都被访问过，返回false
 * 存在还未被访问过的点，返回true
 */
	bool isThereUnvisited() {
		for (int i = 0; i < RawRow; i++) {
			for (int j = 0; j < RawCol; j++) {
				if (!RawArr[i][j].m_isVisited) {
					return true;
				}
			}
		}
		return false;
	}

/*
 * 判断边界
 */
	bool isThePointInBoundary(int x, int y) {
		return x >= 0 && x < RawCol && y >= 0 && y < RawRow;
	}

/*
 * 从指定区间随机出一个数
 */
	double random(double start, double end) {
		return start + (end - start)*rand() / (RAND_MAX + 1.0);
	}

/*
 * 判断这个点的四周（上下左右）是否有通路
 * 判断是否出界 & 是否已经被访问过
 * 返回值为true false，表示是否有通路
 * 如果有通路，那么随机挑选一个能走通的方向，并将参数x, y改变为下一步的坐标
 */
	bool getNeighbor(int & x, int & y) {
		int neighborX[E_Dir_Max];
		int neighborY[E_Dir_Max];
		bool isValid[E_Dir_Max];
		//判断四周是否有通路 => 判断是否出界 & 是否已经被访问过
		//上
		neighborX[E_Dir_Up] = x;
		neighborY[E_Dir_Up] = y - 1;
		isValid[E_Dir_Up] = (isThePointInBoundary(neighborX[E_Dir_Up], neighborY[E_Dir_Up])
			&& !RawArr[neighborY[E_Dir_Up]][neighborX[E_Dir_Up]].m_isVisited);
		
		//下
		neighborX[E_Dir_Down] = x;
		neighborY[E_Dir_Down] = y + 1;
		isValid[E_Dir_Down] = (isThePointInBoundary(neighborX[E_Dir_Down], neighborY[E_Dir_Down])
			&& !RawArr[neighborY[E_Dir_Down]][neighborX[E_Dir_Down]].m_isVisited);
		
		//左
		neighborX[E_Dir_Left] = x - 1;
		neighborY[E_Dir_Left] = y;
		isValid[E_Dir_Left] = (isThePointInBoundary(neighborX[E_Dir_Left], neighborY[E_Dir_Left])
			&& !RawArr[neighborY[E_Dir_Left]][neighborX[E_Dir_Left]].m_isVisited);

		//右
		neighborX[E_Dir_Right] = x + 1;
		neighborY[E_Dir_Right] = y;
		isValid[E_Dir_Right] = (isThePointInBoundary(neighborX[E_Dir_Right], neighborY[E_Dir_Right])
			&& !RawArr[neighborY[E_Dir_Right]][neighborX[E_Dir_Right]].m_isVisited);
		

		if (!(isValid[E_Dir_Up] || isValid[E_Dir_Down] || isValid[E_Dir_Left] || isValid[E_Dir_Right])) {
			return false;	//没有通路，返回false
		}
		else {	//有通路，随机选择一个
			int randnum = random(0, 4);
			int finalDir = 0;
			for (int i = 0; i < E_Dir_Max; i++) {
				if (isValid[i]) {
					finalDir = i;
					break;
				}
			}
			while (randnum) {
				randnum--;
				finalDir++;
				finalDir = finalDir == E_Dir_Max ? 0 : finalDir;
				while (!isValid[finalDir]) {
					finalDir++;
					finalDir = finalDir == E_Dir_Max ? 0 : finalDir;
				}
			}
			x = neighborX[finalDir];
			y = neighborY[finalDir];
			return true;
		}
	}

	glm::vec3 getExit() {
		glm::vec3 exit;
		exit.x = ExitPoint.m_x;
		exit.y = 0.0f;
		exit.z = -ExitPoint.m_y;
		return exit;
	}
};

#endif
