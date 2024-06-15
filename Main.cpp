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
		constexpr Size SIZE{100, 40 };
		/// @brief ブロックの数　縦
		constexpr int Y_COUNT = 5;

		/// @brief ブロックの数　横
		constexpr int X_COUNT = 20;

		/// @brief 合計ブロック数
		constexpr int MAX = Y_COUNT * X_COUNT;
	}
	//パドルの情報
	namespace paddle {
		constexpr Size SIZE{ 60, 10 };
	}
}
class Ball;
class Blocks;
class Paddle;
class GameManager;

//#pragma region アイテムクラス
//class ItemBase {
//public:
//	abstract void Ability() {
//
//	}
//};
//
//#pragma endregion

#pragma region ゲームマネージャーのクラス
class GameManager {
public:
	enum GameMode {
		Start,
		InGame,
		GameClear,
		GameOver,
	};

	GameMode game_mode;

	GameManager() : game_mode(Start) {}

	void StateChange(GameMode change_value) {
		game_mode = change_value;
	}
};
#pragma endregion

#pragma region スコアクラス

class Score {
	const Font font{ FontMethod::MSDF, 48 };
public:
	int score;

	Score() : score(0) {}

	void Reset() {
		score = 0;
	}

	void Draw() {
		font(U"Score:{}"_fmt(score)).draw(25, Vec2{ 25, 25 }, Palette::White);
	}

	void AddScore(int value) {
		score += value;
	}
};
#pragma endregion

#pragma region ボールクラス
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
	Font life_font{ FontMethod::MSDF, 48 };
	int life_count;

	Ball() : velocity({ 0, -constants::ball::SPEED }), ball(400, 400, 8), ball_state(Move), init_pos(400, 400), life_count(3) {}

	void Reset() {
		velocity = { 0, -constants::ball::SPEED };
		ball.setPos(Vec2{ 400, 400 });
		ball_state = Move;
		life_count = 3;
	}

	void Draw() {
		// ボール描画
		ball.draw();
		life_font(U"Life:{}"_fmt(life_count)).draw(25, Vec2{ 500, 25 }, Palette::White);
	}

	void Update(GameManager* gamemanager) {
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
			if (--life_count < 0) {
				gamemanager->StateChange(GameManager::GameOver);
			}
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
#pragma endregion

#pragma region ブロッククラス

class Blocks {
public:
	Rect bricks[constants::brick::MAX];
	Texture block_texture{ U"example/Vantan.png" };
	Texture textures[constants::brick::MAX];
	int block_count;

	Blocks(): block_count(constants::brick::MAX){

		for (int y = 0; y < constants::brick::Y_COUNT; ++y) {
			for (int x = 0; x < constants::brick::X_COUNT; ++x) {
				int index = y * constants::brick::X_COUNT + x;
				textures[index] = block_texture;
				bricks[index] = Rect{
					x * constants::brick::SIZE.x,
					60 + y * constants::brick::SIZE.y,
					constants::brick::SIZE
				};
			}
		}
	}

	void Reset(){

		block_count = constants::brick::MAX;
		for (int y = 0; y < constants::brick::Y_COUNT; ++y) {
			for (int x = 0; x < constants::brick::X_COUNT; ++x) {
				int index = y * constants::brick::X_COUNT + x;
				textures[index] = block_texture;
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
			textures[i].resized(constants::brick::SIZE).drawAt(bricks[i].center());
		}
	}

	void Intersects(Ball* target, Score* score, GameManager* gamemanager) {
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

				score->AddScore(100);
				 //あたったブロックは画面外に出す
				refBrick.y -= 600;
				if (--block_count < 0) {
					gamemanager->StateChange(GameManager::GameClear);
				}
	
				 //同一フレームでは複数のブロック衝突を検知しない
				break;
			}
		}
	}
};
#pragma endregion

#pragma region パドルクラス
class Paddle final { //←ファイナルはこのクラスを継承してはいけないということです
public:
	Rect paddle;
	Paddle() : paddle(Arg::center(Cursor::Pos().x, 500), constants::paddle::SIZE) { }

	void Reset() {
		paddle = { Arg::center(Cursor::Pos().x, 500), constants::paddle::SIZE };
	}

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
#pragma endregion

#pragma region ゲームスタートのクラス
class GameStart {
public:
	Font start_font{ FontMethod::MSDF, 48 };
	Font click_font{ FontMethod::MSDF, 48 };
	Score* score;
	Ball* ball;
	Blocks* blocks;
	Paddle* paddle;

	void Init(Score* s, Ball* b, Blocks* bs, Paddle* p) {
		score = s;
		ball = b;
		blocks = bs;
		paddle = p;
	}

	void Reset() {
		score->Reset();
		ball->Reset();
		blocks->Reset();
		paddle->Reset();
	}

	void Draw(GameManager* gamemanager) {
		start_font(U"Game Start").draw(50, Vec2(250, 200), Palette::Gold);
		click_font(U"Please Left Click").draw(25, Vec2(280, 270), Palette::Limegreen);
		if (MouseL.down()) {
			gamemanager->StateChange(GameManager::InGame);
		}
	}
};
#pragma endregion

#pragma region ゲームクリアのクラス
class GameClear {
public:
	Font end_font{ FontMethod::MSDF, 48 };
	Font click_font{ FontMethod::MSDF, 48 };

	void Draw(GameManager* gamemanager, GameStart* gamestart) {
		end_font(U"Game Clear").draw(50, Vec2(250, 200), Palette::Gold);
		click_font(U"Please Left Click").draw(25, Vec2(280, 270), Palette::Limegreen);
		if (MouseL.down()) {
			gamemanager->StateChange(GameManager::Start);
			gamestart->Reset();
		}
	}
};
#pragma endregion


#pragma region ゲームオーバーのクラス
class GameOver {
public:
	Font end_font{ FontMethod::MSDF, 48 };
	Font click_font{ FontMethod::MSDF, 48 };

	void Draw(GameManager* gamemanager, GameStart* gamestart) {
		end_font(U"Game Over").draw(50, Vec2(250, 200), Palette::Gold);
		click_font(U"Please Left Click").draw(25, Vec2(280, 270), Palette::Limegreen);
		if (MouseL.down()) {
			gamemanager->StateChange(GameManager::Start);
			gamestart->Reset();
		}
	}
};
#pragma endregion






void Main()
{
	Ball ball;
	Blocks blocks;
	Paddle paddle;
	Score score;
	GameStart gamestart;
	GameManager gamemanager;
	GameClear gameclear;
	GameOver gameover;
	gamestart.Init(&score, &ball, &blocks,&paddle);

	while (System::Update())
	{
		switch (gamemanager.game_mode){
		case GameManager::Start:
			gamestart.Draw(&gamemanager);
			break;
		case GameManager::InGame:
			ball.Draw();
			blocks.Draw();
			paddle.Draw();
			score.Draw();
			ball.Update(&gamemanager);
			paddle.Update();
			ball.Intersects(&ball);
			blocks.Intersects(&ball, &score, &gamemanager);
			paddle.Intersects(&ball);
			break;
		case GameManager::GameClear:
			gameclear.Draw(&gamemanager, &gamestart);
			break;
		case GameManager::GameOver:
			gameover.Draw(&gamemanager, &gamestart);
			break;
		default:
			break;
		}
	}
}
