#include <Novice.h>
#include <math.h>
#include <mmsystem.h>
#include <process.h>

#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "winmm.lib")

DWORD WINAPI Threadfunc(void*);
HWND hwMain;
const char kWindowTitle[] = "";

typedef struct {
	float x;
	float y;
} Vector2;

typedef struct {
	Vector2 center;
	float radius;
} Circle;

bool OnCollisionCircle(Circle ca, Circle cb);
void DrawCircle(Circle c, unsigned int color);

Circle a, b;
Vector2 center = { 100, 100 };
char keys[256] = { 0 };
char preKeys[256] = { 0 };
int color = RED;
int speed = 5;

// Windowsアプリエントリーポイント
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	WSADATA wdData;
	static HANDLE hThread;
	static DWORD dwID;

	// ライブラリ初期化
	Novice::Initialize(kWindowTitle, 1024, 768);

	hwMain = GetDesktopWindow();

	a.center.x = 400;
	a.center.y = 400;
	a.radius = 100;

	b.center.x = 200;
	b.center.y = 200;
	b.radius = 50;

	// winsock初期化
	WSAStartup(MAKEWORD(2, 0), &wdData);

	// ソケット通信用スレッド作成
	hThread = (HANDLE)CreateThread(NULL, 0, Threadfunc, (LPVOID)&a, 0, &dwID);

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレーム開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		// 　☆上下左右キー入力されてたら円bの座標を更新（加算値は常識的な値で）
		if (Novice::CheckHitKey(DIK_UP)) {
			b.center.y -= speed; // 上に
		}
		if (Novice::CheckHitKey(DIK_DOWN)) {
			b.center.y += speed; // 下に
		}
		if (Novice::CheckHitKey(DIK_LEFT)) {
			b.center.x -= speed; // 左に
		}
		if (Novice::CheckHitKey(DIK_RIGHT)) {
			b.center.x += speed; // 右に
		}

		// ↓更新処理ここから
		// 　☆2つの円の衝突判定（当たっていたら円bを青で描画）
		if (OnCollisionCircle(a, b)) {
			color = BLUE;
		}
		else {
			color = RED;
		}
		// ↑更新処理ここまで

		// ↓描画処理ここから
		// 　☆2つの円a,bを描画
		DrawCircle(a, WHITE); // a
		DrawCircle(b, color); // b
		// ↑描画処理ここまで

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリ終了
	Novice::Finalize();

	// winsock終了
	WSACleanup();

	return 0;
}

// 通信スレッド関数
DWORD WINAPI Threadfunc(void* px) {

	px;
	SOCKET sWait, sConnect;                // 待機用と接続用
	struct sockaddr_in saLocal, saConnect; // 待機用と接続用
	WORD wPort = 8000;
	int iLen = sizeof(saConnect); // accept関数で使用

	// 待機ソケット作成
	sWait = socket(AF_INET, SOCK_STREAM, 0);

	// 待機ソケットにポート8000番紐づけるbind関数に
	// 引数で渡すSOCKADDR_IN構造体を設定
	ZeroMemory(&saLocal, sizeof(saLocal));
	saLocal.sin_family = AF_INET;
	saLocal.sin_addr.s_addr = INADDR_ANY;
	saLocal.sin_port = htons(wPort);

	if (bind(sWait, (LPSOCKADDR)&saLocal.sin_addr.s_addr, sizeof(saLocal.sin_addr.s_addr)) == SOCKET_ERROR) {
		closesocket(sWait);
		return 1;
	}

	if (listen(sWait, 2) == SOCKET_ERROR) {
		closesocket(sWait);
		return 1;
	}

	// sConnectで接続受け入れ
	sConnect = accept(sWait, (LPSOCKADDR)&saConnect, &iLen);

	if (sConnect == INVALID_SOCKET) {
		shutdown(sConnect, 2);
		closesocket(sConnect);
		shutdown(sWait, 2);
		closesocket(sWait);
		return 1;
	}

	// 接続待ちソケット解放
	shutdown(sWait, 2);
	closesocket(sWait);

	while (1) {
		// データ受信
		int nRcv = recv(sWait, (char*)&sWait, sizeof(Circle), 0);

		if (nRcv == SOCKET_ERROR)break;

		// データ送信
		send(sWait, (const char*)&sWait, sizeof(Circle), 0);
	}

	shutdown(sConnect, 2);
	closesocket(sConnect);

	return 0;
}

// 円同士の衝突判定
bool OnCollisionCircle(Circle ca, Circle cb) {
	float diffA, diffB, c, sumRadius;

	diffA = cb.center.x - ca.center.x;              // Xの差
	diffB = cb.center.y - ca.center.y;              // Yの差
	c = (float)sqrt(diffA * diffA + diffB * diffB); // 平方根
	sumRadius = ca.radius + cb.radius;              // 半径の和

	// cが半径の和以下なら当たっている
	if (c <= sumRadius) {
		return true;
	}

	return false;
}

// 円の描画
void DrawCircle(Circle c, unsigned int cColor) {
	Novice::DrawEllipse(
		(int)c.center.x, (int)c.center.y, // X,Y座標
		(int)c.radius, (int)c.radius,     // X,Y半径
		0.0f,                             // 回転角度
		cColor,                           // 色
		kFillModeSolid                    // 塗りつぶし
	);
}