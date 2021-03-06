#ifndef NNOBJECT_H
#define NNOBJECT_H

/** Nihongo Nano class library
 *
 *      NnObject (オブジェクト)
 *        ┣ NnSortable (比較可能オブジェクト)
 *        ┃   ┗ NnString (文字列)
 *        ┣ NnVector  (可変長配列)
 *        ┣ NnHash (文字列キーのハッシュ)
 *        ┣ NnBullet  (文字列キーと値のペア)
 *        ┣ Writer    (汎出力)
 *        ┃   ┣ FileWriter    (ファイル出力:NYADOS版)
 *        ┃   ┗ StreamWriter  (FILE*出力)
 *        ┃      ┣ FileWriter (ファイル出力:NYACUS版)
 *        ┃      ┗ PipeWriter (パイプ出力)
 *        ┣ Reader    (汎入力)
 *        ┃   ┗ StreamReader  (FILE*入力)
 *        ┃      ┣ FileReader (ファイル入力)
 *        ┃      ┗ PipeReader (パイプ入力)
 *        ┗ NnEnum    (イタレーター)
 *             ┣ NnHash::Each (ハッシュの各要素 → NnBullet )
 *             ┗ NnDir           (ディレクトリ     → NnString )
 *
 * ・delete する権利/義務を「所有権」と呼ぶ。
 *
 * ・コレクション系オブジェクトは被格納オブジェクトに対する
 *   所有権を有する。したがって、auto/static系のオブジェクトは
 *   コレクション系オブジェクトへ登録できない。
 *
 * ・NnObject は、デストラクタを仮想化しており、
 *   コレクション系オブジェクトに格納されるオブジェクトは
 *   NnObject から派生しなくてはいけない。
 *
 * ・引数に渡したオブジェクトに関して、所有権も移動する場合は
 *   ポインタ渡しする。
 *      (例)
 *          NnVector vec;
 *          vec.append( new NnString("hogehoge") );
 */

/* 漢字の先頭バイトであれば、非0 を返す */
#undef  isKanji
#define isKanji(x) ((unsigned char)(((x)^0x20)-0xA1) <= 0x3B)

#undef  isSpace
#define isSpace(x) isspace((x)&255)
#undef  isDigit
#define isDigit(x) isdigit((x)&255)
#undef  isAlpha
#define isAlpha(x) isalpha((x)&255)
    
/* 静的配列の要素を返す */
#undef  numof
#define numof(X) (sizeof(X)/sizeof((X)[0]))

/* 最小の負の値を例外を表す定数とする */
enum{ EXCEPTIONS = (int)(~0u-(~0u >> 1)) };

class NnSortable;

/** NnObject クラス
 *   デストラクタを仮想化した基底クラス。
 *   これを継承すれば、NnVector , NnHash に登録することができる。
 */
class NnObject {
public:
    virtual ~NnObject();
    virtual NnObject   *clone() const;
    virtual NnSortable *sortable();
};

/** NnSortable クラス
 *    比較可能なオブジェクトはこれから継承すると、
 *    NnVector で、sort,uniq メソッドを使うことができる。
 */
class NnSortable : public NnObject {
public:
    /** x と比較して、大小関係を 負・零・正の数値で返す。*/
    virtual int compare( const NnSortable &x) const = 0;
    virtual NnSortable *sortable();
};

/** NnEnum クラス
 *    要素を一つずつ列挙するインターフェイス
 */
class NnEnum : public NnObject {
public:
    /** 次の要素へ移動する */
    virtual void      operator ++ ()=0;
    void next(){ ++*this; }

    /** 現在差している要素へのポインタを返す.
     *  全ての要素を走査し終わると、NULL を返す.
     */
    virtual NnObject *operator *  ()=0;

    /** まだ全ての要素を走査していなければ 真を返す */
    virtual int more();
};

/* このヘッダで宣言されているメソッドの実体は
 * NnString.cpp で定義されている。
 */

template <class T> T max1(T a,T b){ return a<b ? b : a ; }
template <class T> T min1(T a,T b){ return a<b ? a : b ; }

const char *getEnv( const char *var , const char *none=NULL );

#endif

