#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define WS 11   //blocos na horizontal
#define HS 6    //blocos na vertical
#define MAX_BLOCOS 15//quantidade máxima de blocos por fase
float velocidade; //#define não cerve pois tais valores são calculados com base no resultados da função responsividade
float gravidade;
static bool isjumpin=false;
typedef struct{ //coordenadas genericas
    int x; 
    int y;
}cord;
cord coordenadas[WS][HS];   //variavel global pq orientado a objeto em C é uma patifaria
typedef int fases[MAX_BLOCOS+2][2];  //index 0=player, 1=marmita, adiante bloco
typedef struct{ //todas as variaveis referentes a objetos
    Texture2D scarfy;   //sprites da animação
    int animSize;   //tamanho da animação (1 para estaticos)
    Rectangle destRec;  //para movimento e tamanho
    Rectangle sourceRec;    //para animação
    Vector2 origin; //posição de start (0,0 é o canto superior esquerdo, valores negativos movem a direita e positivos a esquerda)
}obj;
obj blocos[MAX_BLOCOS];
typedef struct{ //fases do jogo
    fases fa; //0=ar, 1=parede, 2=player, 3=passa nível
    int constroe;
    int destroe;
}fase;
void declaraObj(obj *o, Texture2D tx, int animSize, int size){
    o->scarfy = tx;    //sprite inicial
    o->animSize = animSize;         //quantos frames a animação possui (1 para estaticos)
    o->destRec.width = (float)size;    
    o->destRec.height = (float)size;
    o->destRec.x=0.0f;
    o->destRec.y=0.0f;
    o->sourceRec.width = (float)tx.width/animSize;
    o->sourceRec.height = (float)tx.height;
    o->sourceRec.x=0.0f;
    o->sourceRec.y=0.0f;
    o->origin.x = 0;   //posição inicial
    o->origin.y = 0;   //(0,0 é o canto superior esquerdo, valores negativos movem a direita e positivos a esquerda)
}
void declaraFase(fase *f, obj *pl, obj *mr, int *cns, int *des, int c, int d, fases cont){
    f->constroe = c;
    f->destroe = d;
    *des=f->destroe;
    *cns=f->constroe;
    pl->destRec.x = coordenadas[cont[0][0]][cont[0][1]].x;
    pl->destRec.y = coordenadas[cont[0][0]][cont[0][1]].y;
    mr->destRec.x = coordenadas[cont[1][0]][cont[1][1]].x;
    mr->destRec.y = coordenadas[cont[1][0]][cont[1][1]].y;
    for (int i = 0; i < 7; i++){
        for(int j=0; j<2; j++){
            f->fa[i][j]= cont[i][j];
        }
    }
    for (int i = 0; i < MAX_BLOCOS; i++) {//aplica as coordenadas aos blocos
        int x = cont[i+2][0];
        int y = cont[i+2][1]; 
        if(x==-1){
            blocos[i].destRec.x=-100*(blocos[i].destRec.width);
        }else if (x >= 0 && x < WS && y >= 0 && y < HS) {
            blocos[i].destRec.x = coordenadas[x][y].x;
            blocos[i].destRec.y = coordenadas[x][y].y;
        }
    }
    
};
void desenhaObj(obj *o){
    DrawTexturePro(o->scarfy, o->sourceRec, o->destRec, o->origin, 0.0f, WHITE);
}
int responsividade(int Wmonitor, int Hmonitor){ //largura monitor, comprimento monitor, quadrados horizontais, quadrados verticais
    int Ssize = (Wmonitor/WS<Hmonitor/HS) ? Wmonitor/WS : Hmonitor/HS; //tamanho dos quadrados
    int Wgrid = Ssize*WS;   //tamanho do grid
    int Hgrid = Ssize*HS;
    int offsetX = (Wmonitor-Wgrid)/2;   //onde o grid começa
    int offsetY = (Hmonitor-Hgrid)/2;
  
    for(int j = 0; j < WS; j++) {
        for(int i = 0; i < HS; i++) {
            coordenadas[j][i].x = offsetX + (j * Ssize);
            coordenadas[j][i].y = offsetY + (i * Ssize);
        }
    }
    return Ssize;//para uso em outras funções
}
void desenhaGrid(int size) {
    for(int i=0; i<WS; i++){    //verticais
        DrawLine(coordenadas[i][0].x,coordenadas[0][0].y,coordenadas[i][0].x,coordenadas[WS-1][HS-1].y+size,BLACK);
    }
    for(int j=0; j<HS; j++){    //horizontais
        DrawLine(coordenadas[0][0].x,coordenadas[0][j].y,coordenadas[WS-1][HS-1].x+size,coordenadas[0][j].y,BLACK);
    }
    DrawLine(coordenadas[WS-1][0].x+size,coordenadas[0][0].y,coordenadas[WS-1][0].x+size,coordenadas[WS-1][HS-1].y+size,BLACK); //linhas finais
    DrawLine(coordenadas[0][0].x,coordenadas[0][HS-1].y+size,coordenadas[WS-1][HS-1].x+size,coordenadas[WS-1][HS-1].y+size,BLACK);
}
void buildBreak(Rectangle colli, Rectangle pl, Rectangle mr, int *cns, int *des){
    static bool click = false;
    if(!click){
    if(IsMouseButtonDown(1) && *des>0){   //botão direito quebra
        Vector2 m = GetMousePosition();
        for(int i=0; i<MAX_BLOCOS; i++){
            if(CheckCollisionPointRec(m, blocos[i].destRec)){
                click=true;
                blocos[i].destRec.x=-1000;
                (*des)-=1;
            }
        }
    }
   
    if(IsMouseButtonDown(0) && *cns>0){   //botão esquerdo constroe
        Vector2 m = GetMousePosition();
        for(int i=0; i<WS; i++){
            for(int j=0; j<HS; j++){
                colli.x=coordenadas[i][j].x;
                colli.y=coordenadas[i][j].y;
                 for(int z=0; z<MAX_BLOCOS; z++){
                if(CheckCollisionPointRec(m,colli) && !CheckCollisionRecs(colli,pl) && !CheckCollisionRecs(colli,mr) && !CheckCollisionPointRec(m,blocos[z].destRec)){
                    click=true;
                    blocos[MAX_BLOCOS-*cns].destRec.x=colli.x;
                    blocos[MAX_BLOCOS-*cns].destRec.y=colli.y;
                    (*cns)-=1;
                    break;
            }
                 }
        }
    }
    }
    }
    if(IsMouseButtonReleased(1) || IsMouseButtonReleased(0)) click=false;   //trava para não construir varias vezes
}
void movePlayer(obj *pl, int speed, bool *r, bool *l){
    
    if (IsKeyDown(KEY_A) && *l){
        pl->destRec.x -= speed*GetFrameTime(); //0.105 representa uma normalização para taxa de frames
        *r=true;    //andar para a direção oposta destrava o andar
    }
    if (IsKeyDown(KEY_D) && *r){
        pl->destRec.x += speed*GetFrameTime();
        *l=true;
    } 
}
void pulo(obj *pl, int *speedV, bool *d, bool *u){
    static float posinicial;
    
    if(IsKeyDown(KEY_W) && *u){
        posinicial = pl->destRec.y;
        *d=false;
        *u=false;  
        isjumpin=true;
    }
    if(*d){
        pl->destRec.y+=velocidade*2*GetFrameTime();   
    }
    if(pl->destRec.y<(posinicial-(pl->destRec.height*2)) && isjumpin){//2blocos
        *d=true;
        isjumpin=false;
        posinicial=coordenadas[0][5].y+100;  
    }
    if(*d) isjumpin=false;
    if(isjumpin){
        pl->destRec.y-=velocidade*2*GetFrameTime(); 
    }
   
    
    
}
void colide(Rectangle pc,Rectangle pe,Rectangle pb,Rectangle pd, bool *u, bool *d, bool *r, bool *l){
    bool coli=false;
    for(int i=0; i<MAX_BLOCOS; i++){
        if(!CheckCollisionRecs(pb,blocos[i].destRec) && !isjumpin){
            *d=true;
        }
        if(!CheckCollisionRecs(pe,blocos[i].destRec)){
            *l=true;
        }
        if(!CheckCollisionRecs(pd,blocos[i].destRec)){
            *r=true;
        }
        if(CheckCollisionRecs(pc,blocos[i].destRec)){
            *u=false;
            *d=true;
            coli=true;
        }
        if(CheckCollisionRecs(pe,blocos[i].destRec)){
            *r=false;  
            coli=true;
        }
        if(CheckCollisionRecs(pb,blocos[i].destRec)){
            *d=false;
            *u=true;
            coli=true;
        }
        if(CheckCollisionRecs(pd,blocos[i].destRec)){
            *l=false;
            coli=true;
           
        }
        if(coli) break;
    }
    if(pb.y>coordenadas[0][5].y+pe.height){
        *d=false;
        *u=true;
    }
    if(pc.y<coordenadas[0][0].y){
        *u=false;
        *d=true;
    }
    if(pd.x<coordenadas[0][0].x){
        *l=false;
    }
    if(pe.x>coordenadas[10][0].x+coordenadas[1][0].x){
        *r=false;
    }
        
}

int main(){
    int monitor = GetCurrentMonitor();  //coleta o monitor especifico que uso
    int Wmonitor = GetMonitorWidth(monitor);    //coleta as proporções de tamanho do monitor
    int Hmonitor = GetMonitorHeight(monitor);
    
    InitWindow(Wmonitor,Hmonitor, "Carlão Aventuras"); //cria uma tela baseada no tamanho do monitor
    ToggleFullscreen(); //arruma as bordas e garante o fullscreen
    Wmonitor = GetMonitorWidth(monitor);    //coleta as proporções de tamanho do monitor
    Hmonitor = GetMonitorHeight(monitor);
    int size=responsividade(Wmonitor, Hmonitor);    //determina o tamanho das coisas
    velocidade=size;//para variar responsivamente
    gravidade=size/4;//para impedir saltos muito altos
    Rectangle colli;    //para colisões com o mouse
    colli.width=size;
    colli.height=size;
    int cns, des;   //recebem dados de fase a cada marmita
    int vel=velocidade; //tais valores são zerados para parar o player, então um backUp é necessário
    int velV=0;
   
    bool r=true, l=true, d=true, u=false;    //para trava de botões de movimentação
    Texture2D bl = LoadTexture("resources/bloco.PNG"), pl = LoadTexture("resources/anda.PNG"), mt = LoadTexture("resources/marmita.PNG");  //declarando texturas
    obj player, marmita;
    declaraObj(&player,pl,4,size);
    declaraObj(&marmita,mt,1,size);
    Rectangle pc,pe,pb,pd;  //colisores do player
    pc.width=(size/2)-10;
    pc.height=10;
    pe.width=10;
    pe.height=size-20;
    pb.width=pc.width;
    pb.height=10;
    pd.width=pe.width;
    pd.height=pe.height;
    int indexFase=0;
    for(int i=0; i<MAX_BLOCOS; i++){
        declaraObj(&blocos[i], bl, 1, size);    
    }
    
    fase F1, F2, F3, F4, F5, F6;
    fases f1 = {
        {0, 3}, //player
        {10, 4}, //marmita
        {0, 5}, //blocos
        {10, 5},
        {5, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1,5}   //para construção, do ultimo até o ultimo-constroe
    }; 
    fases f2 = {
        {0, 3}, //player
        {10, 4}, //marmita
        {0, 5}, //blocos
        {10, 5},
        {5, 5},
        {5, 4},
        {5, 3},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5}   //para construção, do ultimo até o ultimo-constroe
    };
    fases f3 = {
        {0, 3}, //player
        {10, 4}, //marmita
        {0, 5}, //blocos
        {10, 5},
        {5, 5},
        {5, 4},
        {5, 3},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5}    //para construção, do ultimo até o ultimo-constroe
    };
    fases f4 = {
        {0,3},
        {10,2},
        {0,5},  
        {4,3},
        {4,4},
        {4,5},
        {10,1}, 
        {10,3},
        {9,1},
        {9,3},
        {8,1},
        {8,2},  
        {8,3},   //para construção, do ultimo até o ultimo-constroe
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5}
    };
    fases f5 = {
        {0,3},
        {5,1},
        {-1,0},//
        {3,1},
        {7,1},
        {-1,1},
        {-1,2},
        {4,2},
        {5,2},
        {5,3},
        {6,2},
        {-1,2},
        {-1,1},
        {-1,0},
        {-1,0},
        {-1,1},   //para construção, do ultimo até o ultimo-constroe
        {-1, 5}
    };
    fases f6={
        {0, 5},
        {5, 1},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5},
        {-1, 5} 
    };
    declaraFase(&F1,&player,&marmita,&cns,&des,0,0,f1);  //a cada marmita comida
    
    SetTargetFPS(30);
    while (!WindowShouldClose()){   //esc termina
    pc.x=(player.destRec.x+size/3.5f)+10;
    pc.y=player.destRec.y;    
    
    pe.x=pc.x+pc.width;
    pe.y=pc.y+10;
        
    pb.x=pc.x;
    pb.y=player.destRec.y+size-5;

    pd.x=pc.x-10;
    pd.y=pc.y+10;
    
        switch(indexFase){
            case 0:
                DrawText("Use A,W,D para se mover\n\n\nChegue até a marmita!",coordenadas[3][0].x,coordenadas[0][0].y,size/5,RED);  
            break;
            case 1:
                DrawText("Clique esquerdo para construir",coordenadas[3][0].x,coordenadas[0][0].y,size/5,RED);
            break;
            case 2:
                DrawText("Clique direito para destruir",coordenadas[3][0].x,coordenadas[0][0].y,size/5,RED);
            break;
            case 4:
                DrawText("Clap, clap\n\n\nAcabou",coordenadas[3][0].x,coordenadas[0][0].y,size/5,RED);
            break;
            default:
            break;
                
        }
        if(CheckCollisionRecs(player.destRec,marmita.destRec) && indexFase==0){
            declaraFase(&F2,&player,&marmita,&cns,&des,4,0,f2);///////avança fase com colisão
            indexFase++;
            u=false;
            d=true;
        }else if(CheckCollisionRecs(player.destRec,marmita.destRec) && indexFase==1){
            declaraFase(&F3,&player,&marmita,&cns,&des,0,4,f3);///////avança fase com colisão
            indexFase++;
            u=false;
            d=true;
        }else if(CheckCollisionRecs(player.destRec,marmita.destRec) && indexFase==2){
            declaraFase(&F4,&player,&marmita,&cns,&des,1,2,f4);///////avança fase com colisão
            indexFase++;
            u=false;
            d=true;
        }else if(CheckCollisionRecs(player.destRec,marmita.destRec) && indexFase==3){
            declaraFase(&F5,&player,&marmita,&cns,&des,2,1,f5);///////avança fase com colisão
            indexFase++;
            u=false;
            d=true;
        }else if(CheckCollisionRecs(player.destRec,marmita.destRec) && indexFase==4){
            declaraFase(&F6,&player,&marmita,&cns,&des,0,0,f6);///////avança fase com colisão
           
        }
        
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        desenhaGrid(size);
        
        for(int i=0; i<MAX_BLOCOS; i++) desenhaObj(&blocos[i]);
        
        desenhaObj(&player);
        desenhaObj(&marmita);
        buildBreak(colli, player.destRec,marmita.destRec,&cns,&des);
        
        
        movePlayer(&player,vel,&r,&l);
        pulo(&player,&velV,&d,&u);
        colide(pc,pe,pb,pd,&u,&d,&r,&l);
        char str[4];
        sprintf(str, "%d", cns);
        DrawText("Construa:",coordenadas[0][0].x,coordenadas[0][0].y,size/5,RED);
        DrawText(str,coordenadas[0][0].x+2*(size/5),coordenadas[0][0].y+(size/2),size/2,RED);
        sprintf(str, "%d", des);
        DrawText("Destrua:",coordenadas[1][0].x,coordenadas[1][0].y,size/5,RED);
        DrawText(str,coordenadas[1][0].x+2*(size/5),coordenadas[1][0].y+(size/2),size/2,RED);
        DrawText("ESC para\n\nsair",coordenadas[10][0].x,coordenadas[0][0].y,size/5,RED);

        EndDrawing();
        
      
    }

    
    //UnloadTexture(carlos.scarfy);  
    //UnloadTexture(carloss.scarfy);    
    CloseWindow();  //garantia para quando sai do while principal 
    return 0;
}