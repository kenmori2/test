#include <iostream>
#include <curses.h>
#include <unistd.h>
#include <random>

// ゲームの進行速度を調整するためのスリープに用いる値（マイクロ秒）
#define GMCLOCK 60000
// #define GMCLOCK	15000

// ぷよの色を表すの列挙型
// NONEが無し，RED，BLUE，... が色を表す
enum puyocolor
{
    NONE = 0,
    RED = 1,
    BLUE = 2,
    GREEN = 3,
    YELLOW = 4
};

// ぷよぷよの盤面を管理するクラス
// オブジェクト指向言語及び演習１で作成した matrix クラスを継承して作成します
template <class T>
class vector
{
public:
    vector() : num(0), x(NULL)
    {
    }
    vector(int n) : num(n), x(new T[n]())
    {
    }
    ~vector()
    {
        delete[] x;
    }
    vector(const vector &v) : num(v.num), x(num == 0 ? NULL : new T[num])
    {
        for (int i = 0; i < num; i++)
            x[i] = v.x[i];
    }
    virtual void print() const
    {
        if (num == 0)
            std::cout << "Empty" << std::endl;
        else
        {
            for (int i = 0; i < num - 1; i++)
            {
                std::cout << x[i] << " ";
            }
            std::cout << x[num - 1] << std::endl;
        }
    }
    int size() const { return num; }
    T &operator[](int index)
    {
        if (index < 0 || index >= num)
            throw "out of range";
        return x[index];
    }
    const T &operator[](int index) const
    {
        if (index < 0 || index >= num)
            throw "out of range";
        return x[index];
    }
    const vector operator+(const vector &v) const
    {
        vector temp(num);
        for (int i = 0; i < num; i++)
        {
            temp[i] = x[i] + v[i];
        }
        return temp;
    }
    const vector operator-(const vector &v) const
    {
        vector temp(num);
        for (int i = 0; i < num; i++)
        {
            temp[i] = x[i] - v[i];
        }
        return temp;
    }
    void operator=(const vector &v)
    {
        if (this != &v)
        {
            delete[] x;
            num = v.num;
            x = num == 0 ? NULL : new T[num];
            for (int i = 0; i < num; i++)
            {
                x[i] = v.x[i];
            }
        }
    }

protected:
    int num;
    T *x;
};
template <class T>
class matrix : public vector<T>
{
public:
    matrix() : vector<T>(), num1(0), num2(0) {}
    matrix(int r, int c) : vector<T>(r * c), num1(r), num2(c) {}
    matrix(const matrix &m) : vector<T>(m.rows() * m.cols()), num1(m.rows()), num2(m.cols())
    {
        for (int i = 0; i < num1 * num2; i++)
        {
            this->x[i] = m.x[i];
        }
    }

    T &operator()(int r, int c)
    {
        if (r + c * num1 > num1 * num2 - 1 || r + c * num1 < 0)
            throw "out of range";
        return (this->x[r + c * num1]);
    }
    const T &operator()(int r, int c) const
    {
        if (r + c * num1 > num1 * num2 - 1 || r + c * num1 < 0)
            throw "out of range";
        return this->x[c * num1 + r];
    }
    void operator=(const matrix &m)
    {
        if (this != &m)
        {
            delete[] this->x;
            num1 = m.rows();
            num2 = m.cols();
            this->num = num1 * num2;
            this->x = this->num == 0 ? NULL : new T[this->num];
            for (int i = 0; i < num1 * num2; i++)
            {
                this->x[i] = m.x[i];
            }
        }
    }
    void print() const
    {
        if (num1 == 0 && num2 == 0)
            std::cout << "Empty" << std::endl;
        else
        {
            for (int c = 0; c < num2; c++)
            {
                for (int r = 0; r < num1; r++)
                {
                    std::cout << this->x[r + c * num1] << " ";
                }
                std::cout << std::endl;
            }
        }
    }
    int rows() const { return num1; }
    int cols() const { return num2; }

protected:
    int num1;
    int num2;
};

// 講義資料の説明に沿って matrix クラスの機能拡張を済ませておいてください．
class PuyoArray : public matrix<int>
{
public:
    typedef matrix<int> base;

    // デフォルトコンストラクタ（独自のメンバ変数を追加した場合は適宜修正すること）
    PuyoArray() {}
    // 盤面サイズを指定して初期化するコンストラクタ（独自のメンバ変数を追加した場合は適宜修正すること）
    PuyoArray(int rows, int cols) : base(rows, cols), rotate(0), pivotRow(0), pivotCol(0), pivotColor(0), subColor(0) {}
    // コピーコンストラクタ（独自のメンバ変数を追加した場合は適宜修正すること）
    PuyoArray(const PuyoArray &puyo) : base(puyo), rotate(puyo.rotate), pivotCol(puyo.pivotCol), pivotRow(puyo.pivotRow), pivotColor(puyo.pivotColor), subColor(puyo.subColor) {}

public:
    // 代入演算子（独自のメンバ変数を追加した場合は適宜修正すること）
    void operator=(const PuyoArray &puyo)
    {
        if (this != &puyo)
        {
            base::operator=(puyo);
            rotate = puyo.rotate;
            pivotCol = puyo.pivotCol;
            pivotRow = puyo.pivotRow;
            pivotColor = puyo.pivotColor;
            subColor = puyo.subColor;
        }
    }

    // 2つのぷよ配列で同じ位置にぷよが存在する場合は true を返し，
    // 同じ位置にぷよが一つも存在しない場合は false を返す
    bool operator&&(const PuyoArray &puyo) const
    {
        if (puyo.size() != size())
            return false;
        bool flag = false;
        for (int i = 0; i < size(); i++)
        {
            if (puyo[i] != NONE && this->operator[](i) != NONE)
                flag = true;
        }
        return flag;
    }

    // 2つのぷよ配列をマージして新しいぷよ配列を生成
    const PuyoArray operator|(const PuyoArray &puyo) const
    {
        if (puyo.size() != size())
            return *this;
        PuyoArray temp(*this);
        for (int i = 0; i < size(); i++)
        {
            if (puyo[i] != NONE)
                temp[i] = puyo[i];
        }
        return temp;
    }

    // 盤面上のぷよの総数を返す
    int count() const
    {
        const base &mat = *this;
        int num = 0;
        for (int i = 0; i < size(); i++)
        {
            if (mat[i] > 0)
                num++;
        }
        return num;
    }

public:
    // 盤面に新しいぷよ生成
    void GeneratePuyo()
    {
        PuyoArray &active = *this;
        active.rotate = 0;
        // 初期コードはRGのぷよを生成するコードとなっています
        // ランダムなぷよを生成するように修正してください
        active(0, 2) = randomColor();
        active(0, 3) = randomColor();
        active.pivotRow = 0;
        active.pivotCol = 2;
        active.pivotColor = active(0, 2);
        active.subColor = active(0, 3);
    }

    void Gravity(PuyoArray &stack)
    {
        PuyoArray &active = *this;
        for (int c = 0; c < active.cols(); c++)
        {
            for (int r = active.rows() - 1; r >= 0; r--)
            {
                if (active(r, c) != NONE)
                {
                    int above_stack = active.rows() - 1;
                    while (above_stack >= 0 && stack(above_stack, c) != NONE)
                        above_stack--;
                    if (above_stack < 0)
                        continue; // 一番上までスタックが溜まっている時
                    stack(above_stack, c) = active(r, c);
                    active(r, c) = NONE;
                }
            }
        }
    }
    // ぷよの着地判定．着地したぷよの数を返す
    int MoveLandedPuyo(PuyoArray &stack)
    {
        PuyoArray &active = *this; // アクティブ側のぷよ配列
        bool landed = false;
        int num_landed = 0; // 着地したぷよの数を数える変数

        // 1番下に到達したぷよを着地と判定する
        for (int c = 0; c < active.cols(); c++)
        {
            int above_stack = active.rows() - 1;
            while (above_stack >= 0 && stack(above_stack, c) != NONE)
                above_stack--;
            if (above_stack < 0)
                continue; // 一番上までスタックが溜まっている時
            if (active(above_stack, c) != NONE)
            {
                landed = true;
                break;
            }
        }
        if (landed)
        {
            for (int c = 0; c < active.cols(); c++)
            {
                for (int r = active.rows() - 1; r >= 0; r--)
                {
                    if (active(r, c) != NONE)
                        num_landed++;
                }
            }
        }
        return num_landed;
    }
    void Rotate(const PuyoArray &stack)
    {
        PuyoArray &active = *this;
        PuyoArray temp(active);

        int nextRotate = (temp.rotate + 1) % 4;

        int dr[4] = {0, 1, 0, -1};
        int dc[4] = {1, 0, -1, 0};

        int r = temp.pivotRow;
        int c = temp.pivotCol;

        int newSubRow = r + dr[nextRotate];
        int newSubCol = c + dc[nextRotate];

        if (newSubRow < 0 || newSubRow >= temp.rows() || newSubCol < 0 || newSubCol >= temp.cols())
        {
            return;
        }
        if (stack(newSubRow, newSubCol) != NONE)
        {
            return;
        }

        for (int r = 0; r < temp.rows(); r++)
        {
            for (int c = 0; c < temp.cols(); c++)
            {
                temp(r, c) = NONE;
            }
        }

        temp.rotate = nextRotate;
        temp(r, c) = temp.pivotColor;
        temp(newSubRow, newSubCol) = temp.subColor;

        active = temp;
    }
    // 一回の処理で繋がったマスを消してerasedに消したマスを入れる
    void ErasePuyoRec(PuyoArray &erased, int r, int c, int puyocolor)
    {
        if (r < 0 || r >= rows() || c < 0 || c >= cols())
        {
            return;
        }
        if ((*this)(r, c) != puyocolor)
        {
            return;
        }
        erased(r, c) = puyocolor;
        (*this)(r, c) = NONE;
        ErasePuyoRec(erased, r, c + 1, puyocolor);
        ErasePuyoRec(erased, r + 1, c, puyocolor);
        ErasePuyoRec(erased, r, c - 1, puyocolor);
        ErasePuyoRec(erased, r - 1, c, puyocolor);
    }

    void ErasePuyo()
    {
        for (int r = 0; r < rows(); r++)
        {
            for (int c = 0; c < cols(); c++)
            {
                if ((*this)(r, c) == NONE)
                    continue;

                PuyoArray tstack(*this);
                PuyoArray erased(rows(), cols());
                tstack.ErasePuyoRec(erased, r, c, tstack(r, c));
                if (erased.count() >= 4)
                {
                    *this = tstack;
                }
            }
        }
    }
    // 左移動
    void MoveLeft(const PuyoArray &stack)
    {
        PuyoArray &active = *this; // アクティブ側のぷよ配列
        for (int r = 0; r < active.rows(); r++)
        {
            for (int c = 0; c < active.cols(); c++)
            {
                if (active(r, c) != NONE)
                {
                    if (c == 0 || stack(r, c - 1) != NONE)
                        return;
                }
            }
        }
        PuyoArray temp(active);
        temp.pivotCol--;
        // 1つ左の位置にぷよを移動させる
        for (int r = 0; r < temp.rows(); r++)
        {
            for (int c = 1; c < temp.cols(); c++)
            {
                if (temp(r, c) != NONE && temp(r, c - 1) == NONE)
                {
                    temp(r, c - 1) = temp(r, c); // 一つ左にぷよをコピー
                    temp(r, c) = NONE;           // 元の位置のぷよは消す
                }
            }
        }
        // アクティブ側にコピー
        active = temp;
    }

    // 右移動
    void MoveRight(const PuyoArray &stack)
    {
        PuyoArray &active = *this;

        // まず移動可能かチェック
        for (int r = 0; r < active.rows(); r++)
        {
            for (int c = 0; c < active.cols(); c++)
            {
                if (active(r, c) != NONE)
                {
                    if (c == active.cols() - 1)
                        return;
                    if (stack(r, c + 1) != NONE)
                        return;
                }
            }
        }

        PuyoArray temp(active);

        // 右移動なので右から左へ
        for (int r = 0; r < temp.rows(); r++)
        {
            for (int c = temp.cols() - 2; c >= 0; c--)
            {
                if (temp(r, c) != NONE)
                {
                    temp(r, c + 1) = temp(r, c);
                    temp(r, c) = NONE;
                }
            }
        }

        temp.pivotCol++;
        active = temp;
    }

    // 下移動
    void MoveDown(const PuyoArray &stack)
    {
        PuyoArray &active = *this; // アクティブ側のぷよ配列
        PuyoArray temp(active);    // 一時的なぷよ格納配列

        // 1つ下の位置にぷよを移動させる
        for (int r = temp.rows() - 2; r >= 0; r--)
        {
            for (int c = 0; c < temp.cols(); c++)
            {
                if (temp(r, c) != NONE && temp(r + 1, c) == NONE)
                {
                    temp(r + 1, c) = temp(r, c); // 一つ下にぷよをコピー
                    temp(r, c) = NONE;           // 元の位置のぷよは消す
                }
            }
        }
        temp.pivotRow++;
        // アクティブ側にコピー
        active = temp;
    }

private:
    int rotate, pivotRow, pivotCol, pivotColor, subColor;
    int randomColor()
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<int> dist(1, 4);
        return dist(gen);
    }
};

// 矩形を描画
void draw_rectangle(int r1, int c1, int r2, int c2, int color)
{
    attrset(color);
    mvhline(r1, c1, 0, c2 - c1);
    mvhline(r2, c1, 0, c2 - c1);
    mvvline(r1, c1, 0, r2 - r1);
    mvvline(r1, c2, 0, r2 - r1);
    mvaddch(r1, c1, ACS_ULCORNER);
    mvaddch(r2, c1, ACS_LLCORNER);
    mvaddch(r1, c2, ACS_URCORNER);
    mvaddch(r2, c2, ACS_LRCORNER);
}

// 画面表示
void display(const PuyoArray &puyo)
{
    // 枠を描画
    draw_rectangle(1, 0, puyo.rows() + 2, puyo.cols() * 2 + 2, COLOR_PAIR(0));

    // 落下中ぷよ表示
    for (int r = 0; r < puyo.rows(); r++)
    {
        for (int c = 0; c < puyo.cols(); c++)
        {
            switch (puyo(r, c))
            {
            case NONE:
                attrset(COLOR_PAIR(0));
                mvaddch(r + 2, c * 2 + 2, '.');
                break;
            case RED:
                attrset(COLOR_PAIR(1));
                mvaddch(r + 2, c * 2 + 2, 'R');
                break;
            case BLUE:
                attrset(COLOR_PAIR(2));
                mvaddch(r + 2, c * 2 + 2, 'B');
                break;
            case GREEN:
                attrset(COLOR_PAIR(3));
                mvaddch(r + 2, c * 2 + 2, 'G');
                break;
            case YELLOW:
                attrset(COLOR_PAIR(4));
                mvaddch(r + 2, c * 2 + 2, 'Y');
                break;
            default:
                mvaddch(r + 2, c * 2 + 2, '?');
                break;
            }
        }
    }

    // 情報表示
    int count = 0;
    for (int r = 0; r < puyo.rows(); r++)
    {
        for (int c = 0; c < puyo.cols(); c++)
        {
            if (puyo(r, c) != NONE)
            {
                count++;
            }
        }
    }

    // 画面左上にスコアを表示（最初はぷよの数を表示するダミーコード）
    char msg[256];
    snprintf(msg, 256, "Score: %d", count);
    mvaddstr(0, 1, msg);

    refresh();
}

// ここから実行される
int main(int argc, char **argv)
{
    // 画面の初期化
    initscr();

    // カラー属性を扱うための初期化
    start_color();

    // キーを押しても画面に表示しない
    noecho();

    // キー入力を即座に受け付ける
    cbreak();
    curs_set(0);

    // キー入力受付方法指定
    keypad(stdscr, TRUE);

    // キー入力非ブロッキングモード
    timeout(0);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);

    PuyoArray active(12, 6);                       // 縦12マス✕横6マスで初期化
    PuyoArray stack(active.rows(), active.cols()); // activeと同じサイズで初期化

    // 最初のぷよ生成
    active.GeneratePuyo();

    int delay = 0;
    int waitCount = 6;

    // メイン処理ループ
    bool loop = true;
    while (loop)
    {
        // キー入力受付
        int ch = getch();

        // 入力キーごとの処理
        switch (ch)
        {
        case KEY_LEFT:
            active.MoveLeft(stack);
            break;
        case KEY_RIGHT:
            active.MoveRight(stack);
            break;
        case 'q': // q or Q の入力で終了する
        case 'Q':
            loop = false;
            continue;
        case 'x':
            active.Rotate(stack);
        default:
            break;
        }

        // 処理速度調整のためのif文
        if (delay % waitCount == 0)
        {
            if (active.MoveLandedPuyo(stack) > 0)
            {
                // 着地していたら新しいぷよ生成
                active.Gravity(stack);
                active.GeneratePuyo();
            }
            // ぷよ下に移動
            active.MoveDown(stack);

            // ぷよ着地判定
            if (active.MoveLandedPuyo(stack) > 0)
            {
                active.Gravity(stack);
                // 着地していたら新しいぷよ生成
                active.GeneratePuyo();
            }
        }
        delay++;

        // 表示
        display(active | stack);

        // ゲームの進行速度を調整するためのスリープ
        usleep(GMCLOCK);
    }

    // 画面をリセット
    endwin();

    return 0;
}
