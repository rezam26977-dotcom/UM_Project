#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ساختاری برای نگه داشتن مختصات سطر و ستون یک خانه
typedef struct {
    int satr;
    int soton;
} Mokhtasat;

// ساختاری برای نگه داشتن اطلاعات دیوار
typedef struct {
    Mokhtasat m; // مختصات دیوار
    char jahat;  // 'H' برای افقی، 'V' برای عمودی
} Divar;
typedef struct {
    Divar d;    // دیوار
    int omr;    // عمر
} DivarMovaghat;
typedef struct {//برای جوایز فاز5
    Mokhtasat m;
    int gerefteShode; // 0 = نه | 1 = بله
} KhaneSabz;

int davandeGheyreFaal[100];   // 0 = فعال ، 1 = رسیده به هسته ، 2 = گرفته شده توسط شکارچی
int davandeFaal = 0;   //  دونده فعال
int nobatTamamShod = 0;
int gameOver = 0;   // 0 = ادامه، 1 = برد، 1- = باخت
int maxDivarMovaghat;
int countDavande = 0;
int tedadFaal = 0;//برای برد و باخت
int tedadBeHaste = 0;//برای برد و باخت
int tedadKhorde = 0;//برای برد و باخت
int divarjayeze[100]; // برای نگهداری دیوارهای جایزه هر دونده

DivarMovaghat divarMovaghat[100]; // فاز چهار
int tedadDivarMovaghat = 0;

int estefadeDivarMovaghat[100]; // برای هر دونده

// توابع کمکی


// محاسبه فاصله بین دو نقطه
int mohasebeFasele(Mokhtasat a, Mokhtasat b) {
    return abs(a.satr - b.satr) + abs(a.soton - b.soton);  // تابع abs برای قدر مطلق در کتابخانه stdlib.h
}

void hazfDivarAzDivarha(Divar d, int *tedadDivar, Divar divarha[]) {
    for (int i = 0; i < *tedadDivar; i++) {
        if (divarha[i].m.satr == d.m.satr && divarha[i].m.soton == d.m.soton && divarha[i].jahat == d.jahat) {
            // جایگزینی با آخرین دیوار
            divarha[i] = divarha[*tedadDivar - 1];
            (*tedadDivar)--;
            return;
        }
    }
}


int divarMovaghatDarad(Mokhtasat khane, int jahat, int tedadDivarMovaghat0, DivarMovaghat divarMovaghat0[]) {
    for (int i = 0; i < tedadDivarMovaghat0; i++) {
        Divar d = divarMovaghat0[i].d;

        // دیوار افقی
        if (d.jahat == 'H' || d.jahat == 'h') {
            if (jahat == 2 && d.m.satr == khane.satr && d.m.soton == khane.soton)
                return 1;
            if (jahat == 0 && d.m.satr == khane.satr - 1 && d.m.soton == khane.soton)
                return 1;
        }

        // دیوار عمودی
        if (d.jahat == 'V' || d.jahat == 'v') {
            if (jahat == 1 && d.m.satr == khane.satr && d.m.soton == khane.soton)
                return 1;
            if (jahat == 3 && d.m.satr == khane.satr && d.m.soton == khane.soton - 1)
                return 1;
        }
    }
    return 0;
}

// بررسی وجود دیوار
// خروجی: 1 یعنی دیوار هست، 0 یعنی نیست
int divarDarad(Mokhtasat khane, int jahatHarekat, int tedadDivar, Divar divarha[]) {
    //  0 = بالا، 1 = راست، 2 = پایین، 3 = چپ

    for (int i = 0; i < tedadDivar; i++) {
        // دیوار افقی (H)
        if (divarha[i].jahat == 'H' || divarha[i].jahat == 'h') {
            if (jahatHarekat == 2 && divarha[i].m.satr == khane.satr && divarha[i].m.soton == khane.soton) return 1;
            if (jahatHarekat == 0 && divarha[i].m.satr == khane.satr - 1 && divarha[i].m.soton == khane.soton) return 1;
        }
        // دیوار عمودی (V)
        if (divarha[i].jahat == 'V' || divarha[i].jahat == 'v') {
            if (jahatHarekat == 1 && divarha[i].m.satr == khane.satr && divarha[i].m.soton == khane.soton) return 1;
            if (jahatHarekat == 3 && divarha[i].m.satr == khane.satr && divarha[i].m.soton == khane.soton - 1) return 1;
        }//اگر 1 بود یعنی مانعی وجود دارد
    }
    return 0; // مانعی نیست
}
// الگوریتم DFS  (بررسی اتصال)

int dideShode[100][100];//متغیر سراسری
int tedadDideShode = 0;//متغیر سراسری

void jostojoOmghi(Mokhtasat khaneFeli, int kolSatr, int kolSoton, int tedadDivar, Divar divarha[]) {
    dideShode[khaneFeli.satr][khaneFeli.soton] = 1;
    tedadDideShode++;

    int taghirSatr[4] = {-1, 0, 1, 0}; // بالا، راست، پایین، چپ
    int taghirSoton[4] = {0, 1, 0, -1};

    for (int i = 0; i < 4; i++) {
        Mokhtasat badi;
        badi.satr = khaneFeli.satr + taghirSatr[i];
        badi.soton = khaneFeli.soton + taghirSoton[i];

        // 1. چک کردن مرزها
        if (badi.satr >= 0 && badi.satr < kolSatr && badi.soton >= 0 && badi.soton < kolSoton) {
            // 2. بررسی اینکه قبلاً دیده نشده باشد
            if (dideShode[badi.satr][badi.soton] == 0) {
                // 3. بررسی اینکه دیواری بینشان نباشد
                if (divarDarad(khaneFeli, i, tedadDivar, divarha) == 0) {
                    jostojoOmghi(badi, kolSatr, kolSoton, tedadDivar, divarha);
                }
            }
        }
    }
}

int barresiEttesal(int kolSatr, int kolSoton, int tedadDivar, Divar divarha[]) {
    // ریست کردن آرایه دیده شده
    for (int i = 0; i < kolSatr; i++)
        for (int j = 0; j < kolSoton; j++)
            dideShode[i][j] = 0;

    tedadDideShode = 0;
    Mokhtasat shoro = {0, 0};

    // شروع DFS از خانه (0,0)
    jostojoOmghi(shoro, kolSatr, kolSoton, tedadDivar, divarha);

    if (tedadDideShode == (kolSatr * kolSoton)) {
        return 1;
    } else {
        return 0;
    }

}

void zelzele(int satr, int soton, int tedadDavande, Mokhtasat davandeha[], int davandeGheyreFaal0[], int tedadShekarchi,Mokhtasat shekarchiha[], int tedadDivar, Divar divarha[], Mokhtasat haste) {

    int taghirSatr[4] = {-1, 0, 1, 0};
    int taghirSoton[4] = {0, 1, 0, -1};

        // جابه جایی دونده ها
    for (int i = 0; i < tedadDavande; i++) {
        if (davandeGheyreFaal0[i] != 0) continue;

        int jahathayeMojaz[4]; //جهت های مجاز
        int tedadmojaz = 0;

        for (int j = 0; j < 4; j++) {
            Mokhtasat badi = {davandeha[i].satr + taghirSatr[j],davandeha[i].soton + taghirSoton[j]};

            if (badi.satr < 0 || badi.satr >= satr ||badi.soton < 0 || badi.soton >= soton) continue;//خارج از نقشه نباشه

            if (divarDarad(davandeha[i], j, tedadDivar, divarha))
                continue;
            if (divarMovaghatDarad(davandeha[i], j, tedadDivarMovaghat, divarMovaghat))
                continue;
            if (badi.satr == haste.satr && badi.soton == haste.soton)
                continue;//وجود هسته نور

            // دونده نباشه
            int masdod = 0;
            for (int k = 0; k < tedadDavande; k++) {
                if (k == i) continue; // خود دونده رو رد کن حساب نمیشه
                if (davandeGheyreFaal0[k] == 0 &&davandeha[k].satr == badi.satr &&davandeha[k].soton == badi.soton) {
                    masdod = 1;
                    break;
                }
            }
            if (masdod) continue;

            // شکارچی نباشه
            for (int k = 0; k < tedadShekarchi; k++) {
                if (shekarchiha[k].satr == badi.satr &&shekarchiha[k].soton == badi.soton) {
                    masdod = 1;
                    break;
                }
            }
            if (masdod) continue;


            jahathayeMojaz[tedadmojaz] = j;
            tedadmojaz++;
        }

        if (tedadmojaz > 0) {
            int q = rand() % tedadmojaz;
            int entekhab = jahathayeMojaz[q];
            davandeha[i].satr += taghirSatr[entekhab];
            davandeha[i].soton += taghirSoton[entekhab];
        }
    }

    // جابه جایی شکارچی ها
    for (int i = 0; i < tedadShekarchi; i++) {
        int jahathayeMojaz[4];
        int tedadmojaz = 0;

        for (int j = 0; j < 4; j++) {
            Mokhtasat badi = {shekarchiha[i].satr + taghirSatr[j],shekarchiha[i].soton + taghirSoton[j]};

            if (badi.satr < 0 || badi.satr >= satr ||badi.soton < 0 || badi.soton >= soton) continue;

            if (divarDarad(shekarchiha[i], j, tedadDivar, divarha))
                continue;
            if (divarMovaghatDarad(shekarchiha[i], j, tedadDivarMovaghat, divarMovaghat))
                continue;
            if (badi.satr == haste.satr && badi.soton == haste.soton)
                continue;//وجود هسته نور

            // شکارچی نباشه
            int masdod = 0;
            for (int k = 0; k < tedadShekarchi; k++) {
                if (k == i) continue;
                if (shekarchiha[k].satr == badi.satr && shekarchiha[k].soton == badi.soton) {
                    masdod = 1;
                    break;
                }
            }
            if (masdod) continue;

            // دونده نباشه
            for (int k = 0; k < tedadDavande; k++) {
                if (davandeGheyreFaal0[k] == 0 && davandeha[k].satr == badi.satr && davandeha[k].soton == badi.soton) {
                    masdod = 1;
                    break;
                }
            }
            if (masdod) continue;


            jahathayeMojaz[tedadmojaz] = j;
            tedadmojaz++;
        }

        if (tedadmojaz > 0) {
            int q = rand() % tedadmojaz;
            int entekhab = jahathayeMojaz[q];
            shekarchiha[i].satr += taghirSatr[entekhab];
            shekarchiha[i].soton += taghirSoton[entekhab];
        }
    }
}


//صفحه اول
void safhe_shoror_raylib(int toolsafhe, int arzsafhe) {
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("GAME", toolsafhe / 2 - 200, arzsafhe / 2 - 120, 120, GREEN);
        DrawText("PRESS ENTER TO START", toolsafhe / 2 - 220, arzsafhe / 2 + 20, 30, LIME);

        EndDrawing();

        if (IsKeyPressed(KEY_ENTER)) {
            break;
        }
    }
}

// تابع رسم (Raylib)


//خانه های سبز برای جوایز فاز5
KhaneSabz khaneSabz[50];
int tedadKhaneSabz = 0;

int halateroydad = 0;      // 0 = هیچ | 1 تا 4 = حالت رویداد
int TimeNamayeshroydad = 0;  // برای نمایش پیام چند فریم
int darHalatEntakhabSayenegar = 0;//مود4
int sayenegarEntakhabi = 0;//مود4

int satrEntakhab = 0, sotonEntakhab = 0; // مختصات خانه مجاز انتخاب شده
int jahatmod4 = 0;

// تابع انتخاب دونده فعال بعدی
void entekhabDavandeFaal(int tedadDavande) {
    int peyda = 0;
    if (davandeFaal == -1) { // اگر فعلاً نوبت فعالی نداریم
        for (int i = 0; i < tedadDavande; i++) {
            if (davandeGheyreFaal[i] == 0) {
                davandeFaal = i;
                peyda = 1;
                break;
            }
        }
    } else {
        for (int i = 1; i <= tedadDavande; i++) {
            int f= (davandeFaal + i) % tedadDavande;
            if (davandeGheyreFaal[f] == 0) {
                davandeFaal = f;
                peyda = 1;
                break;
            }
        }
    }
    if (!peyda) davandeFaal = -1; // هیچ دونده فعالی باقی نمانده
}

void rasmNaghshe_raylib(int satr, int soton, Mokhtasat haste,int tedadDavande, Mokhtasat davandeha[],int tedadShekarchi, Mokhtasat shekarchiha[],int tedadDivar, Divar divarha[]) {
    const int toolSafhe = 1200;
    const int arzSafhe = 900;
    InitWindow(toolSafhe, arzSafhe, "Game");
    SetTargetFPS(60);

    int hashiye = 80;
    int w = (toolSafhe - 2 * hashiye) / soton;
    int h = (arzSafhe - 2 * hashiye) / satr;
    int size;
    if (w < h) {
        size = w;
    } else {    // اندازه هر خانه مربع
        size = h;
    }


    int startX = (toolSafhe - soton * size) / 2;
    int startY = (arzSafhe - satr * size) / 2;
    int harkatnamoaffaq = 0;
    while (!WindowShouldClose()) {
        if (gameOver == 0) {
            if (darHalatEntakhabSayenegar == 0) {
                int d = davandeFaal;
                if (davandeFaal == -1) {
                    //هیچ دونده فعال نیست
                }
                else if (davandeGheyreFaal[d] == 0) {

                    int jahat = -1;
                    // 1. بررسی کلیدهای حرکتی
                    if (IsKeyPressed(KEY_UP)) jahat = 0;
                    else if (IsKeyPressed(KEY_RIGHT)) jahat = 1;
                    else if (IsKeyPressed(KEY_DOWN)) jahat = 2;
                    else if (IsKeyPressed(KEY_LEFT)) jahat = 3;

                    // الف) اگر کلید حرکتی زده شد
                    if (jahat != -1) {
                        Mokhtasat badi = davandeha[davandeFaal];
                        if (jahat == 0) badi.satr--;
                        if (jahat == 1) badi.soton++;
                        if (jahat == 2) badi.satr++;
                        if (jahat == 3) badi.soton--;

                        int mojaz = 1;
                        // برخورد با مرز
                        if (badi.satr < 0 || badi.satr >= satr || badi.soton < 0 || badi.soton >= soton) {
                            mojaz = 0;
                        }
                        // برخورد با دیوار
                        if (mojaz==1 && divarDarad(davandeha[davandeFaal], jahat, tedadDivar, divarha)) {
                            mojaz = 0;
                        }
                        // برخورد با سایر دونده‌ها
                        for (int i = 0; i < tedadDavande; i++) {
                            if (i == davandeFaal) continue;
                            if (davandeGheyreFaal[i] != 0) continue;

                            if (davandeha[i].satr == badi.satr && davandeha[i].soton == badi.soton) {
                                mojaz = 0;
                                break;
                            }
                        }
                        // بررسی برخورد با شکارچی
                        for (int i = 0; i < tedadShekarchi; i++) {
                            if (shekarchiha[i].satr == badi.satr && shekarchiha[i].soton == badi.soton) {
                                mojaz = 0;
                                harkatnamoaffaq = 1; // نمایش پیام Invalid move
                                break;
                            }
                        }


                        if (mojaz) {
                            davandeha[davandeFaal] = badi;

                            // متغیر کمکی برای مدیریت نوبت در صورت وقوع رویدادهای خاص
                            int ayaNobatTamamShavad = 1;
                            // برخورد با خانه های سبز
                            for (int g = 0; g < tedadKhaneSabz; g++) {
                                if (khaneSabz[g].gerefteShode==0 && khaneSabz[g].m.satr == badi.satr && khaneSabz[g].m.soton == badi.soton) {


                                    halateroydad = (rand() % 4) + 1; // 1 تا 4

                                    TimeNamayeshroydad = 120; // حدود 2 ثانیه

                                    //  حالت 1: نوبت اضافه
                                    if (halateroydad == 1) {
                                        ayaNobatTamamShavad = 0; // نوبت عوض نشود
                                        khaneSabz[g].gerefteShode = 1; // این جایزه گرفته شد

                                    }

                                        //  حالت 2: افزایش دیوار موقت
                                    else if (halateroydad == 2) {
                                        divarjayeze[davandeFaal] += 2;
                                        khaneSabz[g].gerefteShode = 1; // این جایزه گرفته شد

                                    }

                                        //  حالت 3: زلزله
                                    else if (halateroydad == 3) {
                                        zelzele(satr, soton, tedadDavande, davandeha, davandeGheyreFaal,tedadShekarchi, shekarchiha, tedadDivar, divarha, haste);
                                        khaneSabz[g].gerefteShode = 1; // این جایزه گرفته شد
                                    }

                                        // ---- حالت 4: جابه‌جایی شکارچی ----
                                    else if (halateroydad == 4) {
                                        darHalatEntakhabSayenegar = 1; // فعال کردن حالت انتخاب شکارچی
                                        sayenegarEntakhabi = 0; // انتخاب اولین شکارچی
                                        satrEntakhab = shekarchiha[sayenegarEntakhabi].satr;
                                        sotonEntakhab = shekarchiha[sayenegarEntakhabi].soton;
                                        khaneSabz[g].gerefteShode = 1; // این جایزه گرفته شد
                                        ayaNobatTamamShavad = 0;//نوبت عوض نشود فعلا
                                    }


                                    break;
                                }
                            }


                            //  رسیدن به هسته
                            if (badi.satr == haste.satr && badi.soton == haste.soton) {
                                davandeGheyreFaal[davandeFaal] = 1; // رسیده به هسته
                                tedadBeHaste++;

                                nobatTamamShod = 1; // اعلام پایان نوبت تا پایین کد، نوبت عوض شود
                            } else {
                                if (ayaNobatTamamShavad == 1) {
                                    nobatTamamShod = 1;
                                } else {
                                    nobatTamamShod = 0;
                                }
                            }

                            harkatnamoaffaq = 0;
                        } else {
                            harkatnamoaffaq = 1;//برای چاپ حرکت غیر مجاز
                        }


                    }
                    //اگر کلید 0 زده شد نوبت رد شود
                    else if (IsKeyPressed(KEY_ZERO) || IsKeyPressed(KEY_KP_0)) {
                        nobatTamamShod = 1;

                    }
                        // اگر حرکتی نبود و 0 هم زده نشد بررسی دیوار موقت
                    else {
                        char jahatDivarMovaghat = '\0';
                        if (IsKeyPressed(KEY_W)) jahatDivarMovaghat = 'U';
                        else if (IsKeyPressed(KEY_S)) jahatDivarMovaghat = 'D';
                        else if (IsKeyPressed(KEY_A)) jahatDivarMovaghat = 'L';
                        else if (IsKeyPressed(KEY_D)) jahatDivarMovaghat = 'R';

                        if (jahatDivarMovaghat!='\0' && estefadeDivarMovaghat[davandeFaal] < (maxDivarMovaghat + divarjayeze[davandeFaal])) {
                            int ejazeDivar = 1;
                            DivarMovaghat dm;
                            dm.omr = 3; // عمر دیوار
                            Mokhtasat p = davandeha[davandeFaal];

                            // بررسی مرزها برای دیوار
                            if ((jahatDivarMovaghat == 'U' && p.satr == 0) ||(jahatDivarMovaghat == 'D' && p.satr == satr - 1) ||(jahatDivarMovaghat == 'L' && p.soton == 0) ||(jahatDivarMovaghat == 'R' && p.soton == soton - 1)) {
                                ejazeDivar = 0;//روی مرز های نقشه نمیتوان دیوار موقت گذاشت
                            }

                            // تنظیم مختصات دیوار
                            if (jahatDivarMovaghat == 'U') {
                                dm.d.m.satr = p.satr - 1;
                                dm.d.m.soton = p.soton;
                                dm.d.jahat = 'H';
                            } else if (jahatDivarMovaghat == 'D') {
                                dm.d.m.satr = p.satr;
                                dm.d.m.soton = p.soton;
                                dm.d.jahat = 'H';
                            } else if (jahatDivarMovaghat == 'L') {
                                dm.d.m.satr = p.satr;
                                dm.d.m.soton = p.soton - 1;
                                dm.d.jahat = 'V';
                            } else if (jahatDivarMovaghat == 'R') {
                                dm.d.m.satr = p.satr;
                                dm.d.m.soton = p.soton;
                                dm.d.jahat = 'V';
                            }

                            // بررسی تکراری نبودن
                            for (int i = 0; i < tedadDivar; i++) {
                                if (divarha[i].m.satr == dm.d.m.satr && divarha[i].m.soton == dm.d.m.soton && divarha[i].jahat == dm.d.jahat) {
                                    ejazeDivar = 0;
                                    break;
                                }
                            }

                            if (ejazeDivar) {
                                divarha[tedadDivar] = dm.d;
                                tedadDivar++;
                                // بررسی اینکه راه بسته نشود
                                if (barresiEttesal(satr, soton, tedadDivar, divarha)) {
                                    divarMovaghat[tedadDivarMovaghat] = dm;
                                    tedadDivarMovaghat++;
                                    estefadeDivarMovaghat[davandeFaal]++; // کسر از سهمیه بازیکن فعلی
                                    nobatTamamShod = 1;
                                }
                                else {
                                    tedadDivar--; // دیوار بدرد نمیخورد راه رو میبست
                                    // نوبت عوض نمی‌شود تا بازیکن حرکت دیگری انتخاب کند
                                }

                            }
                        }
                    }
                    if (nobatTamamShod) {
                        if (darHalatEntakhabSayenegar == 0) {
                            int ghabli = davandeFaal;

                            entekhabDavandeFaal(tedadDavande);//انتخاب دونده فعال بعدی
                            if (davandeGheyreFaal[ghabli] == 0) {
                                countDavande++;//رفع باگ نوبت دهی وقتی که به هسته نور میرسه
                            }
                        }

                        nobatTamamShod = 0;


                    }


                }

                //  محاسبه تعداد دونده‌های فعال
                int tedadFaalTemp = 0;
                for (int i = 0; i < tedadDavande; i++) {
                    if (davandeGheyreFaal[i] == 0) tedadFaalTemp++;
                }

                // اگر همه دونده‌های فعال نوبتشان تمام شد
                if (countDavande >= tedadFaalTemp && tedadFaalTemp > 0) {//دلیل بزرگتر مساوی اگر به هردلیلی کانت دونده زیاد شد مشکل پیش نیاد
                    // کم شدن عمر دیوارهای موقت (هر نوبت بازیکن)
                    for (int i = 0; i < tedadDivarMovaghat; i++) {
                        divarMovaghat[i].omr--;
                        if (divarMovaghat[i].omr <= 0) {
                            hazfDivarAzDivarha(divarMovaghat[i].d, &tedadDivar, divarha);
                            tedadDivarMovaghat--;
                            divarMovaghat[i] = divarMovaghat[tedadDivarMovaghat];
                            i--;//اونی که ریختیم چک بشه
                        }
                    }
                    //  حرکت سایه‌نگرها اول افقی بعد عمودی درصورتی که فاصله کم شود

                    for (int i = 0; i < tedadShekarchi; i++) {
                        int shekarchiGereft = 0;
                        //  پیدا کردن نزدیک‌ترین نقشه‌بردار
                        int peydaShod = 0;
                        Mokhtasat davandehadaf;
                        int min = 9999;

                        for (int t = 0; t < tedadDavande; t++) {
                            if (davandeGheyreFaal[t] != 0) continue;

                            d = mohasebeFasele(shekarchiha[i], davandeha[t]);
                            if (peydaShod==0 || d < min) {
                                min = d;
                                davandehadaf = davandeha[t];
                                peydaShod = 1;
                            }
                        }
                        for (int j = 0; j < 2 && shekarchiGereft==0; j++) {

                            Mokhtasat shekarchijari = shekarchiha[i];

                            if (peydaShod==0) break;  // هیچ دونده‌ای فعال نیست
                            int harekatAnjamShod = 0;

                            //   تلاش برای حرکت افقی
                            if (shekarchijari.soton != davandehadaf.soton) {

                                int jahatshekarchi;
                                if (shekarchijari.soton < davandehadaf.soton) jahatshekarchi = 1; // راست
                                else jahatshekarchi = 3;                              // چپ

                                Mokhtasat badi = shekarchijari;
                                if (jahatshekarchi == 1) badi.soton++;
                                else badi.soton--;


                                int khaneEshghal = 0;
                                //برخورد نداشتن با شکارچی دیگر
                                for (int k = 0; k < tedadShekarchi; k++) {
                                    if (k == i) continue;

                                    if (shekarchiha[k].satr == badi.satr && shekarchiha[k].soton == badi.soton) {
                                        khaneEshghal = 1;
                                        break;
                                    }
                                }
                                if (badi.satr == haste.satr && badi.soton == haste.soton) {//چک کردن برخورد با هسته
                                    khaneEshghal = 1;
                                }

                                if (khaneEshghal==0 && divarDarad(shekarchijari, jahatshekarchi, tedadDivar, divarha)==0) {
                                    shekarchiha[i] = badi;
                                    harekatAnjamShod = 1;
                                    //چک کردن گرفته شدن دونده ها توسط شکارچی
                                    for (int t = 0; t < tedadDavande; t++) {
                                        if (davandeGheyreFaal[t] != 0) continue;
                                        if (shekarchiha[i].satr == davandeha[t].satr &&shekarchiha[i].soton == davandeha[t].soton) {
                                            davandeGheyreFaal[t] = 2;
                                            //  انتخاب  دونده فعال برای رفع باگ نوبت دهی از تابع استفاده نکردیم
                                            int peyda = 0;
                                            for (int k = 0; k < tedadDavande; k++) {
                                                if (davandeGheyreFaal[k] == 0) {
                                                    davandeFaal = k;
                                                    peyda = 1;
                                                    break;
                                                }
                                            }
                                            if (peyda==0) davandeFaal = -1;
                                            shekarchiGereft = 1;       // توقف حرکت این شکارچی
                                            break;
                                        }
                                    }

                                }
                            }

                            //   اگر افقی نشد تلاش عمودی
                            if (harekatAnjamShod==0 && shekarchijari.satr != davandehadaf.satr) {

                                int jahatshekarchi;
                                if (shekarchijari.satr < davandehadaf.satr) jahatshekarchi = 2; // پایین
                                else jahatshekarchi = 0;                             // بالا

                                Mokhtasat badi = shekarchijari;
                                if (jahatshekarchi == 2) badi.satr++;
                                else badi.satr--;

                                int khaneEshghal = 0;
                                //برخورد نداشتن با شکارچی دیگر
                                for (int k = 0; k < tedadShekarchi; k++) {
                                    if (k == i) continue;

                                    if (shekarchiha[k].satr == badi.satr && shekarchiha[k].soton == badi.soton) {
                                        khaneEshghal = 1;
                                        break;
                                    }
                                }
                                if (badi.satr == haste.satr && badi.soton == haste.soton) {//چک کردن برخورد با هسته
                                    khaneEshghal = 1;
                                }
                                if (khaneEshghal==0 && divarDarad(shekarchijari, jahatshekarchi, tedadDivar, divarha)==0) {
                                    shekarchiha[i] = badi;
                                    harekatAnjamShod = 1;
                                    //چک کردن گرفته شدن دونده ها توسط شکارچی
                                    for (int t = 0; t < tedadDavande; t++) {
                                        if (davandeGheyreFaal[t] != 0) continue;

                                        if (shekarchiha[i].satr == davandeha[t].satr && shekarchiha[i].soton == davandeha[t].soton) {
                                            davandeGheyreFaal[t] = 2;
                                            // انتخاب  دونده فعال
                                            int peyda = 0;
                                            for (int k = 0; k < tedadDavande; k++) {
                                                if (davandeGheyreFaal[k] == 0) {
                                                    davandeFaal = k;
                                                    peyda = 1;
                                                    break;
                                                }
                                            }
                                            if (peyda==0) davandeFaal = -1;
                                            shekarchiGereft = 1;     // توقف حرکت این شکارچی
                                            break;
                                        }
                                    }

                                }

                            }

                        }
                    }
                    countDavande = 0;//ریست کردن کانت دنده
                }
            }
            if (darHalatEntakhabSayenegar == 1) {
                //  منطق کنترل شکارچی در مود 4
                //  انتخاب و جابه‌جایی شکارچی
                    Mokhtasat shekarchiMojaz = shekarchiha[sayenegarEntakhabi];
                    // تغییر شکارچی با TAB
                    if (IsKeyPressed(KEY_TAB)) {
                        sayenegarEntakhabi = (sayenegarEntakhabi + 1) % tedadShekarchi;
                        jahatmod4 = 0;
                        satrEntakhab = shekarchiha[sayenegarEntakhabi].satr;
                        sotonEntakhab = shekarchiha[sayenegarEntakhabi].soton;
                    }

                    // فقط 4 خانه اطراف شکارچی انتخاب شده بررسی می‌شود
                    int taghirSatr[4] = {-1, 0, 1, 0};
                    int taghirSoton[4] = {0, 1, 0, -1};
                    if (IsKeyPressed(KEY_UP)) jahatmod4 = (jahatmod4 + 3) % 4;      // چرخش برعکس
                    if (IsKeyPressed(KEY_DOWN)) jahatmod4 = (jahatmod4 + 1) % 4;        // چرخش ساعت‌گرد
                    if (IsKeyPressed(KEY_LEFT)) jahatmod4 = (jahatmod4 + 3) % 4;
                    if (IsKeyPressed(KEY_RIGHT)) jahatmod4 = (jahatmod4 + 1) % 4;

                    satrEntakhab = shekarchiMojaz.satr + taghirSatr[jahatmod4];
                    sotonEntakhab = shekarchiMojaz.soton + taghirSoton[jahatmod4];

                    //اعمال حرکت با ENTER
                    if (IsKeyPressed(KEY_ENTER)) {

                        Mokhtasat maghsad = {satrEntakhab, sotonEntakhab};
                        int mojaz = 1;

                        //  بررسی مرز
                        if (maghsad.satr < 0 || maghsad.satr >= satr || maghsad.soton < 0 || maghsad.soton >= soton) {
                            mojaz = 0;
                        }

                        // دیوار
                        if (mojaz==1 && divarDarad(shekarchiMojaz, jahatmod4, tedadDivar, divarha)==1) {//دیوار وجود دارد
                            mojaz = 0;
                        }

                        //  برخورد با شکارچی دیگر
                        if (mojaz==1) {
                            for (int i = 0; i < tedadShekarchi; i++) {
                                if (i == sayenegarEntakhabi) continue;
                                if (shekarchiha[i].satr == maghsad.satr && shekarchiha[i].soton == maghsad.soton) {
                                    mojaz = 0;
                                    break;
                                }
                            }
                        }
                        //  برخورد با دونده فعال
                        if (mojaz==1) {
                            for (int i = 0; i < tedadDavande; i++) {
                                if (davandeGheyreFaal[i] != 0) continue;
                                if (davandeha[i].satr == maghsad.satr && davandeha[i].soton == maghsad.soton) {
                                    mojaz = 0;
                                    break;
                                }
                            }
                        }

                        //  اگر همه شرایط اوکی بود انتقال شکارچی
                        if (mojaz==1) {
                            shekarchiha[sayenegarEntakhabi] = maghsad;
                            darHalatEntakhabSayenegar = 0;  // خروج از مود 4
                            halateroydad = 0;
                            nobatTamamShod = 1;
                            harkatnamoaffaq = 0;
                        } else {
                            harkatnamoaffaq = 1; // نمایش Invalid move
                        }
                    }


            }


            // برد و باخت
            tedadFaal = 0;
            tedadKhorde = 0;
            tedadBeHaste = 0;
            for (int i = 0; i < tedadDavande; i++) {
                if (davandeGheyreFaal[i] == 0) tedadFaal++;
                else if (davandeGheyreFaal[i] == 1) tedadBeHaste++;
                else if (davandeGheyreFaal[i] == 2) tedadKhorde++;
            }

            // حداقل لازم برای برد
            int hadaghal = (tedadDavande + 2) / 3;//گرد شده به سمت بالا

            // شرط برد
            if (tedadBeHaste >= hadaghal) {
                gameOver = 1;//برد
            }

                // شرط باخت:
                // هیچ دونده فعالی نمانده و هنوز به حداقل نرسیده‌ایم
            else if ((tedadFaal == 0 && tedadBeHaste < hadaghal) || (tedadFaal + tedadBeHaste < hadaghal)) {
                gameOver = -1;//باخت
            }


        }

        BeginDrawing();
        ClearBackground(WHITE);
        if (gameOver == 1) {
            DrawText("YOU WIN!", 400, 40, 40, GREEN);
        } else if (gameOver == -1) {
            DrawText("GAME OVER", 380, 40, 40, RED);
        }
        //  پیام جوایز
        if (TimeNamayeshroydad > 0 && gameOver == 0) {
            char msg[64];
            if (halateroydad == 0) sprintf(msg, "EVENT: MODE %d", halateroydad + 1);//نشان میداد مود 0
            else {
                sprintf(msg, "EVENT: MODE %d", halateroydad);
            }
            DrawText(msg, toolSafhe / 2 - 120, 10, 30, DARKGREEN);
            TimeNamayeshroydad--;
        }

        if (harkatnamoaffaq==1 && gameOver == 0) {
            DrawText("Invalid move", 350, 10, 35, RED);
        }




        // رسم خطوط جدول
        for (int i = 0; i <= satr; i++)
            DrawLine(startX, startY + i * size, startX + soton * size, startY + i * size, LIGHTGRAY);
        for (int j = 0; j <= soton; j++)
            DrawLine(startX + j * size, startY, startX + j * size, startY + satr * size, LIGHTGRAY);

            //رسم دیوار ها دیوار اصلی مشکی و دیوار موقت خاکستری
        for (int i = 0; i < tedadDivar; i++) {
            int x = startX + divarha[i].m.soton * size;
            int y = startY + divarha[i].m.satr * size;

            //  تشخیص دیوار موقت
            Color rangDivar = BLACK;
            for (int j = 0; j < tedadDivarMovaghat; j++) {
                if (divarMovaghat[j].d.m.satr == divarha[i].m.satr && divarMovaghat[j].d.m.soton == divarha[i].m.soton && divarMovaghat[j].d.jahat == divarha[i].jahat) {
                    rangDivar = GRAY;   // دیوار موقت
                    break;
                }
            }


            if (divarha[i].jahat == 'H' || divarha[i].jahat == 'h') {
                DrawLineEx((Vector2) {(float) x, (float) (y + size)}, (Vector2) {(float) (x + size), (float) (y + size)}, 6,rangDivar);
            } else {
                DrawLineEx((Vector2) {(float)( x + size), (float) y}, (Vector2) {(float) (x + size), (float) (y + size)}, 6,rangDivar);
            }
        }

        // رسم کادر ضخیم دور نقشه
        DrawRectangleLinesEx((Rectangle) {(float) startX, (float) startY, (float) (soton * size), (float) (satr * size)}, 6,DARKGRAY);

        // هسته (طلایی)ّ
        DrawRectangle(startX + haste.soton * size + size / 4, startY + haste.satr * size + size / 4, size / 2, size / 2,GOLD);

        //  رسم خانه‌های سبز
        for (int i = 0; i < tedadKhaneSabz; i++) {
            if (khaneSabz[i].gerefteShode == 0) {
                DrawRectangle(startX + khaneSabz[i].m.soton * size + size / 4,startY + khaneSabz[i].m.satr * size + size / 4, size / 2, size / 2, GREEN);
            }
        }

        // دونده‌ها
        for (int i = 0; i < tedadDavande; i++) {
            Color c;
            if (davandeGheyreFaal[i] == 2) continue;
            if (davandeGheyreFaal[i] == 1) continue;
            if (i == davandeFaal) c = DARKBLUE;
            else c = BLUE;

            DrawCircle(startX + davandeha[i].soton * size + size / 2, startY + davandeha[i].satr * size + size / 2,(float)(size / 3.0), c);
            // نمایش شماره نقشه‌بردار روی خودش
            char matn[4];
            sprintf(matn, "%d", i + 1);   // اگر خواستی از 1 شروع شود

            int pahnayematn = MeasureText(matn, size / 3);

            DrawText(matn, startX + davandeha[i].soton * size + size / 2 - pahnayematn / 2,startY + davandeha[i].satr * size + size / 2 - (size / 6), size / 3, WHITE);

        }


        // شکارچی‌ها (قرمز)
        for (int i = 0; i < tedadShekarchi; i++) {
            Color c = RED;
            // در مود 4 فاز 5 هنگام انتخاب شکارچی رنگش یکم تیره تر میشه
            if (darHalatEntakhabSayenegar==1 && i == sayenegarEntakhabi) {
                c = MAROON;
            }
            DrawCircle(startX + shekarchiha[i].soton * size + size / 2, startY + shekarchiha[i].satr * size + size / 2,(float)(size / 3.0), c);
        }

        // حلقه انتخاب خانه مقصد
        if (darHalatEntakhabSayenegar==1) {
            // کادر دور شکارچی انتخاب شده
            DrawRectangleLinesEx((Rectangle) {(float) (startX + shekarchiha[sayenegarEntakhabi].soton * size),(float) (startY + shekarchiha[sayenegarEntakhabi].satr * size),(float) size, (float) size}, 5, YELLOW);

            // کادر دور خانه مقصد مجاز سبز غیر مجاز قرمز میشه
            Color rangHalghe;
            if (harkatnamoaffaq==1)
                rangHalghe = RED;
            else
                rangHalghe = GREEN;
            DrawRectangleLinesEx((Rectangle) {(float) (startX + sotonEntakhab * size),(float) (startY + satrEntakhab * size),(float) size, (float) size}, 4, rangHalghe);


        }

        // به‌روزرسانی تعداد دونده‌های فعال و خورده شده
        tedadFaal = 0;
        tedadKhorde = 0;
        tedadBeHaste = 0; // اضافه کنیم تا همیشه دقیق باشد

        for (int i = 0; i < tedadDavande; i++) {
            if (davandeGheyreFaal[i] == 0) tedadFaal++;
            else if (davandeGheyreFaal[i] == 1) tedadBeHaste++;
            else if (davandeGheyreFaal[i] == 2) tedadKhorde++;
        }

        char ettelaat[128]; //اطلاعاتّ
        sprintf(ettelaat, "Be Haste: %d\nActive: %d\nEaten: %d", tedadBeHaste, tedadFaal, tedadKhorde);
        DrawText(ettelaat, toolSafhe - 150, 70, 20, BLACK);//نمایش امار در گوشه راست بازی


        DrawText("GAME", 20, 20, 20, DARKGRAY);
        //نمایش تعداد دیوارهای موقت
        DrawText("Runner Temporary Wall:", 20, 50, 20, DARKGRAY);

        for (int i = 0; i < tedadDavande; i++) {
            char ettelaat0[32];
            sprintf(ettelaat0, "%d: %d / %d", i + 1, estefadeDivarMovaghat[i], maxDivarMovaghat + divarjayeze[i]);

            Color matnColor;

            if (i == davandeFaal) {
                matnColor = BLUE;
            } else {
                matnColor = DARKGRAY;
            }


            DrawText(ettelaat0, 20, 80 + i * 25, 20, matnColor);
        }

        EndDrawing();
    }
    CloseWindow();
}

void saveGame(int satr, int soton,int tedadDavande, Mokhtasat davandeha[], int davandeGheyreFaal0[],int tedadShekarchi, Mokhtasat shekarchiha[],
        Mokhtasat haste,int tedadDivar, Divar divarha[],int tedadDivarMovaghat0, DivarMovaghat divarMovaghat0[],int tedadKhaneSabz0,
        KhaneSabz khaneSabz0[],int maxDivar, int maxDivarMovaghat0,int counterDavande0, int davandeFaal0, int gameover) {
    FILE *f = fopen("savegame.txt", "w");
    if (f==0) return;

    fprintf(f, "%d %d\n", satr, soton);
    fprintf(f, "%d %d %d\n", tedadDavande, tedadShekarchi, maxDivar);
    fprintf(f, "%d %d\n", haste.satr, haste.soton);

    for (int i = 0; i < tedadDavande; i++)
        fprintf(f, "%d %d %d\n", davandeha[i].satr, davandeha[i].soton, davandeGheyreFaal0[i]);

    for (int i = 0; i < tedadShekarchi; i++)
        fprintf(f, "%d %d\n", shekarchiha[i].satr, shekarchiha[i].soton);

    fprintf(f, "%d\n", tedadDivar);
    for (int i = 0; i < tedadDivar; i++)
        fprintf(f, "%d %d %c\n", divarha[i].m.satr, divarha[i].m.soton, divarha[i].jahat);

    fprintf(f, "%d\n", tedadDivarMovaghat0);
    for (int i = 0; i < tedadDivarMovaghat0; i++)
        fprintf(f, "%d %d %c %d\n",divarMovaghat0[i].d.m.satr,divarMovaghat0[i].d.m.soton,divarMovaghat0[i].d.jahat,divarMovaghat0[i].omr);

    fprintf(f, "%d\n", tedadKhaneSabz0);
    for (int i = 0; i < tedadKhaneSabz0; i++)
        fprintf(f, "%d %d %d\n",khaneSabz0[i].m.satr,khaneSabz0[i].m.soton,khaneSabz0[i].gerefteShode);

    fprintf(f, "%d %d %d\n", maxDivarMovaghat0, counterDavande0, davandeFaal0);
    fprintf(f, "%d", gameover);

    fclose(f);
}

int loadGame(int *satr, int *soton,int *tedadDavande, Mokhtasat davandeha[], int davandeGheyreFaal0[],int *tedadShekarchi, Mokhtasat shekarchiha[],
        Mokhtasat *haste,int *tedadDivar, Divar divarha[],int *tedadDivarMovaghat0, DivarMovaghat divarMovaghat0[],int *tedadKhaneSabz0, KhaneSabz khaneSabz0[],
        int *maxDivar, int *maxDivarMovaghat0,int *counterDavande0, int *davandeFaal0, int *gameover) {
    FILE *f = fopen("savegame.txt", "r");
    if (!f) return 0; // یعنی بازی قبلی وجود ندارد

    fscanf(f, "%d %d", satr, soton);
    fscanf(f, "%d %d %d", tedadDavande, tedadShekarchi, maxDivar);
    fscanf(f, "%d %d", &haste->satr, &haste->soton);

    for (int i = 0; i < *tedadDavande; i++)
        fscanf(f, "%d %d %d", &davandeha[i].satr, &davandeha[i].soton, &davandeGheyreFaal0[i]);

    for (int i = 0; i < *tedadShekarchi; i++)
        fscanf(f, "%d %d", &shekarchiha[i].satr, &shekarchiha[i].soton);

    fscanf(f, "%d", tedadDivar);
    for (int i = 0; i < *tedadDivar; i++)
        fscanf(f, "%d %d %c", &divarha[i].m.satr, &divarha[i].m.soton, &divarha[i].jahat);

    fscanf(f, "%d", tedadDivarMovaghat0);
    for (int i = 0; i < *tedadDivarMovaghat0; i++)
        fscanf(f, "%d %d %c %d",&divarMovaghat0[i].d.m.satr,&divarMovaghat0[i].d.m.soton,&divarMovaghat0[i].d.jahat,&divarMovaghat0[i].omr);

    fscanf(f, "%d", tedadKhaneSabz0);
    for (int i = 0; i < *tedadKhaneSabz0; i++)
        fscanf(f, "%d %d %d",&khaneSabz0[i].m.satr,&khaneSabz0[i].m.soton,&khaneSabz0[i].gerefteShode);

    fscanf(f, "%d %d %d", maxDivarMovaghat0, counterDavande0, davandeFaal0);
    fscanf(f, "%d", gameover);

    fclose(f);
    return 1;
}


// تابع main

int main() {
    srand(time(NULL));
    int satr, soton, tedadDavande, tedadShekarchi, maxDivar;
    Mokhtasat davandeha[100];
    Mokhtasat shekarchiha[100];
    Divar divarha[1000];
    Mokhtasat haste;
    int tedadDivar = 0;
    printf("Do you want to continue previous game? (1=yes / 0=no): ");
    int javab; //جواب 1 ادامه 0 خیر
    scanf("%d", &javab);

    int loaded = 0;

    if (javab == 1) {
        loaded = loadGame(&satr, &soton,&tedadDavande, davandeha, davandeGheyreFaal,&tedadShekarchi, shekarchiha,&haste,
                &tedadDivar, divarha,&tedadDivarMovaghat, divarMovaghat,&tedadKhaneSabz, khaneSabz,
                &maxDivar, &maxDivarMovaghat,&countDavande, &davandeFaal, &gameOver);
        if (gameOver != 0) {//برای اینه که اگر بازی برد یا باخت اعلام شده بود ادامه ان نیاید
            gameOver = 0;
            loaded = 0;
        }
        if (loaded == 0) {
            printf("No previous save found. Starting new game.\n");
        }
    }

    if (loaded == 0) {
        do {
            printf("Enter Rows Cols: ");
            scanf("%d %d", &satr, &soton);
            if (satr < 1 || soton < 1 || satr > 16 || soton > 16)printf("The game dimensions must be between 1 and 16.\n");
        } while (satr < 1 || soton < 1 || satr > 16 || soton > 16);
        int kochektarinbodnaqshe; // برای محاسبه تعداد دیوار موقت ها

        if (satr < soton) {
            kochektarinbodnaqshe = satr;
        } else {
            kochektarinbodnaqshe = soton;
        }


        if (kochektarinbodnaqshe / 3 < 1) {
            maxDivarMovaghat = 1;
        } else {
            maxDivarMovaghat = kochektarinbodnaqshe / 3;
        }

        do {
            printf("Enter Runners and Hunters and MaxWalls: ");
            scanf("%d %d %d", &tedadDavande, &tedadShekarchi, &maxDivar);
            if (tedadDavande == 0) { printf("<<<<The number of runners cannot be zero!>>>>\n"); }
            if(tedadDavande+tedadShekarchi>=satr*soton){ printf("The number of runners and hunters is not allowed.\n");}

        } while (tedadDavande == 0||tedadDavande+tedadShekarchi>=satr*soton);

        const int toolSafhe = 1200;
        const int arzSafhe = 900;


        InitWindow(toolSafhe, arzSafhe, "Game");
        // صفحه اول بازی
        safhe_shoror_raylib(1200, 900);


        // تولید هسته
        haste.satr = rand() % satr;
        haste.soton = rand() % soton;


        //  1. تولید دیوارها با منطق DFS
        while (tedadDivar < maxDivar) {
            Divar d;
            d.m.satr = rand() % satr;
            d.m.soton = rand() % soton;
            if (rand() % 2 == 0) {
                d.jahat = 'H';
            } else {
                d.jahat = 'V';
            }


            // جلوگیری از بستن مرزها
            if (d.jahat == 'H' && d.m.satr == satr - 1) { continue; }
            if (d.jahat == 'V' && d.m.soton == soton - 1) { continue; }
            // بررسی تکراری نبودن دیوار
            int tekrari = 0;
            for (int j = 0; j < tedadDivar; j++) {
                if (divarha[j].m.satr == d.m.satr && divarha[j].m.soton == d.m.soton && divarha[j].jahat == d.jahat) {
                    tekrari = 1;
                    break;
                }
            }
            if (tekrari == 1) continue;
            // اضافه کردن موقت به آرایه
            divarha[tedadDivar] = d;
            tedadDivar++;

            // بررسی اتصال DFS
            if (barresiEttesal(satr, soton, tedadDivar, divarha) == 0) {
                tedadDivar--;
            }
        }

        //   تولید شکارچی‌ها
        int tedadShekarchiSakhte = 0;

        while (tedadShekarchiSakhte < tedadShekarchi ) {
            Mokhtasat sh;
            sh.satr = rand() % satr;
            sh.soton = rand() % soton;

            if (mohasebeFasele(sh, haste) < 2) continue;
                // بررسی تکراری نبودن
                int tekrari = 0;
                for (int j = 0; j < tedadShekarchiSakhte; j++) {
                    if (shekarchiha[j].satr == sh.satr && shekarchiha[j].soton == sh.soton) {
                        tekrari = 1;
                        break;
                    }
                }
                if (tekrari==1) continue;

                shekarchiha[tedadShekarchiSakhte] = sh;
                tedadShekarchiSakhte++;

        }

        //تولید دونده ها
        int tedadDavandeSakhte = 0;

        while (tedadDavandeSakhte < tedadDavande) {
            Mokhtasat dav;
            dav.satr = rand() % satr;
            dav.soton = rand() % soton;

            if (mohasebeFasele(dav, haste) < 2) continue;

            int kheiliNazdik = 0;
            for (int h = 0; h < tedadShekarchi; h++) {
                if (mohasebeFasele(dav, shekarchiha[h]) < 2) {
                    kheiliNazdik = 1;
                    break;
                }
            }
            if (kheiliNazdik==1) continue;

            // بررسی تکراری نبودن
            int tekrari = 0;
            for (int j = 0; j < tedadDavandeSakhte; j++) {
                if (davandeha[j].satr == dav.satr && davandeha[j].soton == dav.soton) {
                    tekrari = 1;
                    break;
                }
            }
            if (tekrari==1) continue;

            davandeha[tedadDavandeSakhte] = dav;
            tedadDavandeSakhte++;
        }
        //مقداردهی شمارنده دیوار موقت
        for (int i = 0; i < tedadDavande; i++) {
            estefadeDivarMovaghat[i] = 0;
            divarjayeze[i] = 0;
        }
        davandeFaal = 0;
        countDavande = 0;
        for (int i = 0; i < tedadDavande; i++) {
            davandeGheyreFaal[i] = 0;   // همه دونده‌ها فعالند
        }
        //خانه های سبز
        tedadKhaneSabz = (satr * soton) / 10; // حدود 10٪ نقشه

        for (int i = 0; i < tedadKhaneSabz; i++) {
            int tekrari;
            do {
                tekrari = 0;
                khaneSabz[i].m.satr = rand() % satr;
                khaneSabz[i].m.soton = rand() % soton;
                khaneSabz[i].gerefteShode = 0;

                // چک کردن تداخل با هسته
                if (khaneSabz[i].m.satr == haste.satr && khaneSabz[i].m.soton == haste.soton) {
                    tekrari = 1;
                    continue;
                }

                // چک کردن تداخل با دونده‌ها
                for (int j = 0; j < tedadDavande; j++) {
                    if (khaneSabz[i].m.satr == davandeha[j].satr && khaneSabz[i].m.soton == davandeha[j].soton) {
                        tekrari = 1;
                        break;
                    }
                }
                if (tekrari==1) continue;

                // چک کردن تداخل با شکارچی‌ها
                for (int j = 0; j < tedadShekarchi; j++) {
                    if (khaneSabz[i].m.satr == shekarchiha[j].satr && khaneSabz[i].m.soton == shekarchiha[j].soton) {
                        tekrari = 1;
                        break;
                    }
                }
                if (tekrari==1) continue;

                // چک کردن تداخل با دیگر خانه‌های سبز
                for (int j = 0; j < i; j++) {
                    if (khaneSabz[i].m.satr == khaneSabz[j].m.satr && khaneSabz[i].m.soton == khaneSabz[j].m.soton) {
                        tekrari = 1;
                        break;
                    }
                }


            } while (tekrari==1);
        }

    }



    // شروع نمایش نقشه
    rasmNaghshe_raylib(satr, soton, haste,tedadDavande, davandeha,tedadShekarchi, shekarchiha,tedadDivar, divarha);

    saveGame(satr, soton,tedadDavande, davandeha, davandeGheyreFaal,tedadShekarchi, shekarchiha,haste,
             tedadDivar, divarha,tedadDivarMovaghat, divarMovaghat,
             tedadKhaneSabz, khaneSabz,maxDivar, maxDivarMovaghat,
             countDavande, davandeFaal, gameOver);


    return 0;
}