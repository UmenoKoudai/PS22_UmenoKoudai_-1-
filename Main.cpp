# include <Siv3D.hpp> // Siv3D v0.6.14

namespace constants {
	//ボールの情報
	namespace ball {
		/// @brief ボールの速さ
		constexpr double SPEED = 480.0;
	}
	//ブロックの情報
	
	namespace brick {
		/// @brief ブロックのサイズ
		constexpr Size SIZE{ 40, 20 };
		/// @brief ブロックの数　縦
		constexpr int Y_COUNT = 5;

		/// @brief ブロックの数　横
		constexpr int X_COUNT = 20;

		/// @brief 合計ブロック数
		constexpr int MAX = Y_COUNT * X_COUNT;
	}

	namespace paddle {
		constexpr Size SIZE{ 60, 10 };
	}
}

class Ball {
public:
	enum BallState {
		Move,
		Respawn,
	};
	Vec2 velocity;
	Vec2 init_pos;
	Circle ball;
	BallState ball_state;

	Ball() : velocity({ 0, -constants::ball::SPEED }), ball(400, 400, 8), ball_state(Move), init_pos(400, 400) {}

	void Draw() {
		// ボール描画
		ball.draw();
	}

	void Update() {
		 //ボール移動　↓これを使わないとSiv3Dの物理挙動にならない
		switch (ball_state){
		case Ball::Move:
			ball.moveBy(velocity * Scene::DeltaTime());
			if (ball.y > 600) {
				ball_state = Respawn;
			}
			break;
		case Ball::Respawn:
			ball.setPos(init_pos);
			ball_state = Move;
			velocity = { 0, -constants::ball::SPEED };
			break;
		default:
			break;
		}
	}

	void Intersects(Ball* target) {
		 //天井との衝突を検知
		if ((target->ball.y < 0) && (target->velocity.y < 0))
		{
			target->velocity.y *= -1;
		}

		 //壁との衝突を検知
		if (((target->ball.x < 0) && (target->velocity.x < 0))
			|| ((Scene::Width() < ball.x) && (0 < target->velocity.x)))
		{
			target->velocity.x *= -1;
		}
	}
};

class Blocks {
public:
	Rect bricks[constants::brick::MAX];
	Blocks(){
		for (int y = 0; y < constants::brick::Y_COUNT; ++y) {
			for (int x = 0; x < constants::brick::X_COUNT; ++x) {
				int index = y * constants::brick::X_COUNT + x;
				bricks[index] = Rect{
					x * constants::brick::SIZE.x,
					60 + y * constants::brick::SIZE.y,
					constants::brick::SIZE
				};
			}
		}
	}

	void Draw() {
		 //ブロック描画
		for (int i = 0; i < constants::brick::MAX; ++i) {
			bricks[i].stretched(-1).draw(HSV{ bricks[i].y - 40 });
		}
	}

	void Intersects(Ball* target) {
		 //ブロックとの衝突を検知
		for (int i = 0; i < constants::brick::MAX; ++i) {
			auto& refBrick = bricks[i];

			if (refBrick.intersects(target->ball)) {

				 //ブロックの上辺、または底辺と交差
				if (refBrick.bottom().intersects(target->ball) || refBrick.top().intersects(target->ball))
				{
					target->velocity.y *= -1;
				}
				else // ブロックの左辺または右辺と交差
				{
					target->velocity.x *= -1;
				}

				 //あたったブロックは画面外に出す
				refBrick.y -= 600;
	
				 //同一フレームでは複数のブロック衝突を検知しない
				break;
			}
		}
	}
};

class Paddle final { //←ファイナルはこのクラスを継承してはいけないということです
public:
	Rect paddle;
	Paddle() : paddle(Arg::center(Cursor::Pos().x, 500), constants::paddle::SIZE) { }

	void Draw() const { //←コンストを書く理由は関数の中で数値の変更そさせないようにするため
		 //パドル描画(rounded => 角を丸くさせる)
		paddle.rounded(3).draw();
	}

	void Update() {
		 //パドル
		paddle = Rect{ Arg::center(Cursor::Pos().x, 500),constants::paddle::SIZE };
	}

	void Intersects(Ball* target) {
		 //パドルとの衝突を検知
		if ((0 < target->velocity.y) && paddle.intersects(target->ball))
		{
			target->velocity = Vec2{
				(target->ball.x - paddle.center().x) * 10,
				-target->ball.y
			}.setLength(constants::ball::SPEED);
		}
	}
};

void Main()
{
	Ball ball;
	Blocks blocks;
	Paddle paddle;

	while (System::Update())
	{
		ball.Draw();
		blocks.Draw();
		paddle.Draw();
		ball.Update();
		paddle.Update();
		ball.Intersects(&ball);
		blocks.Intersects(&ball);
		paddle.Intersects(&ball);
	}
}
