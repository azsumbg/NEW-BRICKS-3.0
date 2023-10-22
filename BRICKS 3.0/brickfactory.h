#pragma once

#ifdef BRICKFACTORY_EXPORTS
#define BRICKFACTORY_API _declspec(dllexport)
#else 
#define BRICKFACTORY_API _declspec(dllimport)
#endif

//SCREEN 800 x 600

enum class dirs {
	stop = 0, pad_dir_center = 1, left = 2, right = 3, up = 4, down = 5, up_left = 6,
	up_right = 7, down_left = 8, down_right = 9, pad_dir_right = 10, pad_dir_left = 11
};

enum class bricks { border = 0, cerramic = 1, gold = 2, normal = 3, silver = 4, stone = 5, fall = 6 };

enum class balls { normal = 0, fire = 1, bullet = 2 };

enum class pads { normal = 0, big = 1, shooter = 2, net = 3 };

// CLASSES ////////////////////////////////

class BRICKFACTORY_API BASE
{
	protected:
		float width = 0;
		float height = 0;

	public:
		float x = 0;
		float y = 0;
		float ex = 0;
		float ey = 0;

		BASE(float __x = 0, float __y = 0, float __width = 1.0f, float __height = 1.0f)
		{
			x = __x;
			y = __y;
			width = __width;
			height = __height;
			ex = x + width;
			ey = y + height;
		}

		virtual ~BASE() {};

		void SetEdges()
		{
			ex = x + width;
			ey = y + height;
		}

		void NewDims(float __new_width, float __new_height)
		{
			width = __new_width;
			height = __new_height;
			ex = x + width;
			ey = y + height;
		}

		virtual void Release()
		{
			delete this;
		}

};

class BRICKFACTORY_API BRICK_BASE : public BASE
{
	protected:
		float speed = 0.5f;

	public:

		bricks type = bricks::border;
		int lifes = 0;

		BRICK_BASE(float __where_x, float __where_y) :BASE(__where_x, __where_y, 40.0f, 40.0f) {};

		virtual ~BRICK_BASE() {};

		virtual bool Move() = 0;

		virtual void Transform() = 0;

		virtual void Release() = 0;
};

class BRICKFACTORY_API PAD : public BASE
{
	protected:
		float speed = 20.0f;

	public:
		pads type = pads::normal;
		dirs dir = dirs::stop;
		int net_counter = 500;

		PAD(float __where_x, float __where_y, pads __what_pad = pads::normal) :BASE(__where_x, __where_y, 90.0f, 23.0f)
		{
			if (__what_pad == pads::net)
			{
				NewDims(800.0f, 40.0f);
				type = pads::net;
				net_counter = 1000;
			}
		};

		virtual ~PAD () {};

		bool Move(dirs __to_where)
		{
			switch (__to_where)
			{
				case dirs::left:
					if (x - speed >= 0.0f)
					{
						x -= speed;
						SetEdges();
						return true;
					}
					break;

				case dirs::right:
					if (ex + speed <= 800.0f)
					{
						x += speed;
						SetEdges();
						return true;
					}
					break;
			}
			return false;
		}

		void Transform(pads __which_pad)
		{
			switch (__which_pad)
			{
				case pads::normal:
					type = pads::normal;
					NewDims(90.0f, 23.0f);
					break;

				case pads::big:
					type = pads::big;
					NewDims(150.0f, 30.0f);
					break;

				case pads::shooter:
					type = pads::shooter;
					NewDims(100.0f, 19.0f);
					break;
			}
		}

		void Release()
		{
			delete this;
		}
};

class BRICKFACTORY_API BALL :public BASE
{
	protected:
		float speed = 3.0f;
		int count = 0;
		int delay = 8;

	public:
		balls type = balls::normal;
		dirs dir = dirs::up;
		float lambda = 0;
		int strenght = 1;

		BALL(float __wherex, float __wherey, balls __type = balls::normal) :BASE(__wherex, __wherey, 20.0f, 20.0f) 
		{
			if (__type == balls::bullet)
			{
				type = balls::bullet;
				NewDims(15.0f, 15.0f);
			}
		};

		virtual ~BALL() {};

		int Frame()
		{
			delay--;
			if (delay <= 0)
			{
				delay = 8;
				count++;
				if (count > 3)count = 0;
			}
			return count;
		}

		bool Move()
		{
			if (type == balls::bullet)
			{
				if (ey - 5.0f + speed <= 50.0f)
				{
					y -= 5.0f + speed;
					SetEdges();
					return true;
				}
				else return false;
			}
			else
			{
				switch (dir)
				{
					case dirs::up:
						if (y - speed >= 50.0f)
						{
							y -= speed;
							SetEdges();
							return true;
						}
						else
						{
							dir = dirs::down;
							return true;
						}
						break;

					case dirs::down:
						if (y + speed <= 550.0f)
						{
							y += speed;
							SetEdges();
							return true;
						}
						break;

					case dirs::left:
						if (x - speed >= 0.0f)
						{
							x -= speed;
							SetEdges();
							return true;
						}
						else
						{
							dir = dirs::up_right;
							return true;
						}
						break;

					case dirs::right:
						if (ex + speed <= 800.0f)
						{
							x += speed;
							SetEdges();
							return true;
						}
						else
						{
							dir = dirs::up_left;
							return true;
						}
						break;

					case dirs::up_left:
						if (y - speed >= 50.0f && x - speed >= 0.0f)
						{
							y -= speed;
							x -= speed;
							SetEdges();
							return true;
						}
						else
						{
							if (y - speed < 50.0f && x - speed < 0.0f)dir = dirs::down_right;
							else if (x - speed < 0.0f)dir = dirs::up_right;
							else if (y - speed < 50.0f)dir = dirs::down_left;
							return true;
						}
						break;

					case dirs::up_right:
						if (y - speed >= 50.0f && ex + speed <= 800.0f)
						{
							y -= speed;
							x += speed;
							SetEdges();
							return true;
						}
						else
						{
							if (y - speed < 50.0f && ex + speed > 800.0f)dir = dirs::down_left;
							else if (ex + speed > 800.0f)dir = dirs::up_left;
							else if (y - speed < 50.0f)dir = dirs::down_right;
							return true;
						}
						break;

					case dirs::down_left:
						if (y + speed <= 550.0f && x - speed >= 0.0f)
						{
							x -= speed;
							y += speed;
							SetEdges();
							return true;
						}
						else
						{
							if (y + speed > 550.0f)break;
							else if (x - speed < 0.0f)
							{
								dir = dirs::down_right;
								return true;
							}
						}
						break;

					case dirs::down_right:
						if (y + speed <= 550.0f && ex + speed <= 800.0f)
						{
							x += speed;
							y += speed;
							SetEdges();
							return true;
						}
						else
						{
							if (y + speed > 550.0f)break;
							else if (ex + speed > 800.0f)
							{
								dir = dirs::down_left;
								return true;
							}
						}
						break;

					case dirs::pad_dir_center:
						dir = dirs::up;
						break;

					case dirs::pad_dir_left:
						if(y - speed >= 50.0f && x - speed * lambda >= 0.0f)
						{
							y -= speed;
							x -= speed * lambda;
							SetEdges();
							return true;
						}
						else
						{
							if (y - speed < 50.0f && x - speed * lambda < 0.0f)dir = dirs::down_right;
							else if (x - speed * lambda < 0.0f)dir = dirs::up_right;
							else if (y - speed < 50.0f)dir = dirs::down_left;
							return true;
						}
						break;

					case dirs::pad_dir_right:
						if (y - speed >= 50.0f && ex + speed * lambda <= 800.0f)
						{
							y -= speed;
							x += speed * lambda;
							SetEdges();
							return true;
						}
						else
						{
							if (y - speed < 50.0f && ex + speed * lambda > 800.0f)dir = dirs::down_left;
							else if (ex + speed * lambda > 800.0f)dir = dirs::up_left;
							else if (y - speed < 50.0f)dir = dirs::down_right;
							return true;
						}
						break;
				}

			}
			return false;
		}

		void Transform()
		{
			if (type == balls::normal)
			{
				type = balls::fire;
				NewDims(20.0f, 15.0f);
				strenght = 3;
				count = 0;
			}
			else
			{
				type = balls::normal;
				NewDims(20.0f, 20.0f);
				strenght = 1;
			}
		}

		void Release()
		{
			delete this;
		}
};  

// typedefs ///////////////////////////////

typedef BASE* BaseObj;

typedef BRICK_BASE* BrickObj;

typedef PAD* PadObj;

typedef BALL* BallObj;

extern BRICKFACTORY_API BrickObj iCreateBrick(float _where_x, float _where_y, bricks _which_type);