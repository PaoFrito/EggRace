#include <Adafruit_GFX.h>
#include <Arduino_ST7789_Fast.h>
#include <EEPROM.h>

#define TFT_DC    7
#define TFT_RST   8
#define SCR_WD   240
#define SCR_HT   240

Arduino_ST7789 lcd = Arduino_ST7789(TFT_DC, TFT_RST);

// Variaveis de controle do menu
int menuCtrl = 0;
int menuCtrlAnterior = 0;

// Nome do jogo
char GameName[] = "EggRace";

// Struct que representa objetos do jogo (obstaculos, player, objetivo)
struct Object {
  float x;
  float y;
  int width;
  int height;
};

// Struct do rank
struct Rank {
  char user[3];
  int pts;
};

//Velocidade do jogo
float VelObjetos, VelObjetivo, VelLinhas;


void setup() {
  Serial.begin(9600);
  lcd.init(SCR_WD, SCR_HT);
  pinMode(2, INPUT_PULLUP);
  Menu(menuCtrl);
  //Limpar EEPROM
  //for(int i = 0; i < 20 ; i++) EEPROM.write(i, 0);
}

void loop() {

  // Mapeia a veriavel de controle de acordo com o potenciometro
  menuCtrl = map(analogRead(A0), 0, 1023, 0, 4);
  if (digitalRead(2) == LOW) { // Aperto do botão
    if (menuCtrl == 0) {
      Jogar();
    }

    else if (menuCtrl == 1) {
      showRankings();
      while (true) {
        if (digitalRead(2) == LOW)
          break;
        delay(20);
      }
      delay(1000);
    }

    else if (menuCtrl == 2) {
      showCredits();
      while (true) {
        if (digitalRead(2) == LOW)
          break;
        delay(20);
      }
      delay(1000);
    }

    else if (menuCtrl == 3) {
      showHistory();
      while (true) {
        if (digitalRead(2) == LOW)
          break;
        delay(20);
      }
      delay(1000);
    }

    menuCtrl = map(analogRead(A0), 0, 1023, 0, 4);
    menuCtrlAnterior = menuCtrl;
    Menu(menuCtrl);
  }

  if (menuCtrl != menuCtrlAnterior) {
    Menu(menuCtrl);
    menuCtrlAnterior = menuCtrl;
  }

  delay(20);
}

// Desenha o menu
void Menu(int opc) {
  drawHeader(GameName, 60);
  Escrever("Jogar", 60, 80, 3, WHITE, 0 - opc);
  Escrever("Ranking", 60, 110, 3, WHITE, 1 - opc);
  Escrever("Creditos", 60, 140, 3, WHITE, 2 - opc);
  Escrever("Historia", 60 , 170, 3, WHITE, 3 - opc);

  lcd.fillRect(40, 80 + 30 * opc + 10, 5, 5, RED);
}

//Funcao para escrever os cabecalhos
void drawHeader(char *text, int x) {
  lcd.fillScreen(BLACK);
  for (int i = 0; i < 21; i++) {
    lcd.fillRect(i * 20, 0, 10, 2, YELLOW);
    lcd.fillRect(i * 20, 50, 10, 2, YELLOW);
  }
  Escrever(text, x, 15, 3, WHITE, 1);
}

// Desenha qualquer texto
void Escrever(char *text, int X, int Y, int FontSize, char FontColor, int selected) {
  lcd.setCursor(X, Y);
  lcd.setTextColor(FontColor);
  lcd.setTextSize(FontSize);
  if (!selected)
    lcd.setTextColor(YELLOW);
  lcd.println(text);
}

// JOGAR
void Jogar() {
  // Vetor que guarda os obstáculos
  Object objetos[5];
  // Variável do player
  Object player;
  // Variável do objetivo
  Object objetivo;
  // Faixas da estrada
  Object linhas[3][5];

  int genCount = 0;
  int genCd = 10;
  int objsCount = 0;
  int pontos = 0;
  //Controla a cada quanto tempo o jogador ganhara pontos;
  int ptsTime = 15;
  int pontosCd = 0;

  VelObjetos = 10;
  
  //Gera as calçadas da tela
  lcd.fillScreen(BLACK);
  lcd.fillRect(0, 0, 30, 240, GREY);
  lcd.fillRect(210, 0, 30, 240, GREY);
  lcd.fillRect(0, 0, 240, 30, BLACK);

  //Gera as faixas da pista
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 5; j++) {
      linhas[i][j].x = 72 + (48 * i);
      linhas[i][j].y = (32 * j) + ((j - 1) * 48);
      linhas[i][j].width = 1;
      linhas[i][j].height = 48;
    }
  }

  player.x = 31;
  player.y = 190;
  player.width = 25;
  player.height = 45;

  objetivo.x = 30 + (random(0, 2) * 138);
  objetivo.y = 30;
  objetivo.width = 40;
  objetivo.height = 45;

  int xAnterior = player.x;
  bool run = true;

  lcd.drawFastVLine(0, 0, 30, WHITE);
  lcd.drawFastVLine(239, 0, 30, WHITE);
  Escrever("Pontos: ", 4 , 4, 2, WHITE, 1);
  
  while (run) {
    drawBG(linhas);
    drawObjects(objetos, objsCount, player, objetivo);
    moveObjects(objetos, objsCount, objetivo);
    moveBG(linhas);

    xAnterior = player.x;

    // Muda a posição do jogador de acordo com o potenciometro
    player.x = map(analogRead(A0), 40, 1023, 30, 185);

    if (objsCount < 3) { // Gera os Obstaculos
      if (genCount == 0) {
        objetos[objsCount].x = 42 + (random(0, 4) * 48);
        objetos[objsCount].y = 32;
        objetos[objsCount].width = 25;
        objetos[objsCount].height = 45;
        genCount = 1;
        objsCount++;
      } else {
        if (genCount >= genCd) {
          genCount = 0;
        } else {
          genCount++;
        }
      }
    }
    //Limita para um máximo de 10fps
    delay(100);

    clearObjects(objetos, objsCount, xAnterior, linhas, objetivo);

    if(pontosCd % 10 == 0){ //Incrementa e desenha a pontuacao a cada 1 seg
      pontos = attPts(pontos);
      //Aumenta a velocidade (dificuldade) a cada 5 seg
      if(pontosCd == 100){VelObjetos++;pontosCd = 0;}
    }
    pontosCd++;
    
    lcd.drawFastHLine(0, 0, 240, WHITE);
    lcd.drawFastHLine(0, 25, 240, WHITE);
    
    if (checkColisions(objetos, objsCount, player, objetivo, pontos))
      break;
  }
  gameOver(pontos);
}

//Atualiza os pontos na tela
int attPts(int pts){
  char chr[50];
  String str = "";
  Escrever(chr, 120 , 3, 2, BLACK, 1);
  pts++;  
  str += pts;
  str.toCharArray(chr, 50);
  Escrever("Pontos: ", 4 , 4, 2, WHITE, 1);
  Escrever(chr, 120 , 3, 2, WHITE, 1);
  return pts;
}
      

// funcao que desenha as linhas da estrada
void drawBG(Object linhas[][5]) {
  for (int i = 0; i < 3; i++) {
    for (int j = 1; j < 5; j++) {
      if (linhas[i][j].y > 30)
        lcd.fillRect(linhas[i][j].x, linhas[i][j].y, linhas[i][j].width, linhas[i][j].height, YELLOW);
    }
  }
}

// Move as linhas da estrada
void moveBG(Object linhas[][5]) {
  for (int i = 0; i < 3; i++) {
    for (int j = 1; j < 5; j++) {
      linhas[i][j].y += VelObjetos * 1.25;
      if (linhas[i][j].y > 240) {
        linhas[i][j].y = -29;
      }
    }
  }
}

// Desenha o jogador, o objetivo e os carros
void drawObjects(Object objs[], int lenght, Object player, Object objetivo) {
  if (objetivo.y > 30)
    lcd.fillRect(objetivo.x, objetivo.y, objetivo.width, objetivo.height, RGBto565(9, 102, 2));
  for (int i = 0; i < lenght; i++) {
    if (objs[i].y > 30)
      lcd.fillRect(objs[i].x, objs[i].y, objs[i].width, objs[i].height, RED);
  }
  lcd.fillRect(player.x, player.y, player.width, player.height, BLUE);
}

void moveObjects(Object objs[], int lenght, Object &objetivo) { // funcao que move os carros e o objetivo
  for (int i = 0; i < lenght; i++) {
    objs[i].y += VelObjetos;
    if (objs[i].y > 250) {
      objs[i].x = 42 + (random(0, 4) * 48);
      objs[i].y = 31;
    }
  }

  //velocidade relativa a velocidade dos obstaculos
  objetivo.y += VelObjetos * 1.25;
  if (objetivo.y > 280) {
    objetivo.x = 30 + (random(0, 2) * 138);
    objetivo.y = 31;
  }
}

// funcao que desenha de preto a pos. anterior dos objetos
void clearObjects(Object objs[], int lenght, int xAnterior, Object linhas[][5], Object objetivo) {
  for (int i = 0; i < lenght; i++) {
    if (objs[i].y > 30)
      lcd.fillRect(objs[i].x, objs[i].y - VelObjetos, objs[i].width, objs[i].height, BLACK);
  }
  lcd.fillRect(xAnterior, 190, 25, 45, BLACK);
  for (int i = 0; i < 3; i++) {
    for (int j = 1; j < 5; j++) {
      if (linhas[i][j].y > 30)
        lcd.fillRect(linhas[i][j].x, linhas[i][j].y - (VelObjetos * 1.25), linhas[i][j].width, linhas[i][j].height, BLACK);
    }
  }
  lcd.fillRect(objetivo.x, objetivo.y - (VelObjetos * 1.25), objetivo.width, objetivo.height, BLACK);
}

// checa a colisao do player com os objetos
bool checkColisions(Object object[], int lenght, Object player, Object &objetivo, int &pontos) { 
  bool col = false;
  
  for (int i = 0; i < lenght; i++) { // Colisao com carros
    if (player.x < object[i].x + object[i].width && player.x + player.width > object[i].x &&
        player.y < object[i].y + object[i].height && player.y + player.height > object[i].y) {
      col = true;
    }
  }

  // colisao com o objetivo
  if (player.x < objetivo.x + objetivo.width && player.x + player.width > objetivo.x &&
      player.y < objetivo.y + objetivo.height && player.y + player.height > objetivo.y) {
      objetivo.x = 30 + (random(0, 2) * 138);
      objetivo.y = -30;
      pontos += 10;
      attPts(pontos);
  }
  
  return col;
}

void gameOver(int pts) { // funcao que aciona depois de perder o jogo
  lcd.fillScreen(BLACK);
  Escrever("GAME OVER", 40 , 100, 3, WHITE, 1);
  delay(1000);
 lcd.fillScreen(BLACK);
  menuReturn(menuCtrl, pts);
  
  while (true) { // Mais opcao para o jogador ...
    menuCtrl = map(analogRead(A0), 0, 1023, 0, 2);
    if (digitalRead(2) == LOW) {
      if (menuCtrl == 0) { // retorna ao menu principal
        break;
      }
      if (menuCtrl == 1) { // Uploada do rank
        uploadRank(pts);
        break;
      }
    }

    if (menuCtrl != menuCtrlAnterior) {
      menuReturn(menuCtrl, pts);
      menuCtrlAnterior = menuCtrl;
    }
    delay(20);
  }
}

// Desenha a funcao de retorno
void menuReturn(int opc, int pts) {
  drawHeader("Game Over", 45);
  char chr[50];
  String str = "";
  str += pts;
  str.toCharArray(chr, 50);
  Escrever("Pontos: ", 4 , 200, 2, WHITE, 1);
  Escrever(chr, 120 , 200, 2, WHITE, 1);
  Escrever("Voltar menu", 60, 80, 2, WHITE, 0 - opc);
  Escrever("Gravar pontos", 60, 110, 2, WHITE, 1 - opc);

  lcd.fillRect(40, 80 + 30 * opc + 10, 5, 5, RED);
}

// Funcao de upload do rank
void uploadRank(int pts) {
  char user[3] = "AAA";
  int count = 0;
  int mapChar = map(analogRead(A0), 0, 1023, 65, 91);
  int mapCharAnterior = mapChar;
  user[count] = mapChar;
  showUpload(count, user);
  Rank rank; // guarda o rank do usuario
  rank.pts = pts;
  Serial.println(pts);

  while (count < 3) {
    int mapChar = map(analogRead(A0), 0, 1023, 64, 91); // mapeia as letras, relacionando o potenciometro com o codigo ASCII

    if (digitalRead(2) == LOW) { // Seleciona a letra atual no aperto do botao
      user[count] = mapChar;
      count++;

      mapChar = map(analogRead(A0), 0, 1023, 64, 91);
      mapCharAnterior = mapChar;
      user[count] = mapChar;
      showUpload(count, user);

      if (count == 3)
        break;
    }

    if (mapChar != mapCharAnterior) {
      user[count] = mapChar;
      showUpload(count, user);
      mapCharAnterior = mapChar;
    }
    delay(200);
  }

  Rank otherRanks[5];
  for (int i = 0; i < 3; i++) {
    rank.user[i] = user[i];
  }

  int addr = 0;
  for (int i = 0; i < 5; i++) { // Carrega os ranks salvos no EEPROM
    for (int j = 0; j < 3; j++) {
      otherRanks[i].user[j] = EEPROM.read(addr);
      Serial.print(otherRanks[i].user[j]);
      addr++;
    }
    otherRanks[i].pts = EEPROM.read(addr);
    Serial.println(otherRanks[i].pts);
    addr++;
  }

  int menor = rank.pts;
  Serial.print(rank.user);
  Serial.println(rank.pts);

  addr = -1;
  for (int i = 0; i < 5; i++) { // Caso a pontuacao seja maior que o top 5 sera salva no EEPROM
    if (otherRanks[i].pts < menor) {
      menor = otherRanks[i].pts;
      addr = 4 * i;
    }
  }
  if (addr >= 0) { // Salva no EEPROM
    for (int i = 0; i < 3; i++) {
      EEPROM.write(addr + i, rank.user[i]);
    }
    EEPROM.write(addr + 3, rank.pts);
  }

}

void showUpload(int count, char *user) { // Desenha as letras que o usuario seleciona
  lcd.fillScreen(BLACK);
  drawHeader("Game Over", 45);
  for (int i = 0; i < 3; i++) {
    lcd.setCursor(90 + (20 * i), 110);
    if (i == count)
      lcd.setTextColor(RED);
    else
      lcd.setTextColor(YELLOW);
    lcd.setTextSize(3);
    lcd.println(user[i]);
  }
}

// RANKING

void showRankings() {
  Rank ranks[5];
  int addr = 0;

  //Lendo dados do EEPROM
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 3; j++) {
      //Lendo cada caractere do nome do usuario
      ranks[i].user[j] = EEPROM.read(addr);
      Serial.print(ranks[i].user[j]);
      addr++;
    }
    //Lendo Pontuacao
    ranks[i].pts = EEPROM.read(addr);
    Serial.print(ranks[i].pts);
    addr++;
  }

  //Selection Sort Para arranjar o array da pontuacao
  int maior = 0;
  for (int i = 0; i < 5; i++) {
    maior = i;
    for (int j = i + 1; j < 5; j++) {
      if (ranks[j].pts > ranks[maior].pts)
        maior = j;
    }

    Rank temp = ranks[maior];
    ranks[maior] = ranks[i];
    ranks[i] = temp;
  }

  drawHeader("Rankings", 45);

  //Printa na tela os rankings
  for (int i = 0; i < 5; i++) {
    String str;
    for (int j = 0; j < 3; j++)
      str += ranks[i].user[j];
    str += " ";
    str += ranks[i].pts;
    char chr[50];
    str.toCharArray(chr, 50);
    Escrever(chr, 60, 80 + 30 * i, 3, WHITE, 0);
  }

}

// CREDITOS

void showCredits() {

  drawHeader("Creditos", 45);
  Escrever("Fares", 60, 80, 3, WHITE, 0);
  Escrever("Henrique", 60, 110, 3, WHITE, 0);
  Escrever("Alysson", 60, 140, 3, WHITE, 0);
  Escrever("Gabriel", 60 , 170, 3, WHITE, 0);

}

void showHistory() {

  drawHeader("Historia", 45);

  char text[] =
    "Seu Ze e um trabalhador independente, seu sustento"
    "vem da venda diarias de ovos que ele faz."
    " Com seu negocio crescendo, seu ze decide te"
    "contratar para ajuda-lo com as entregas. Com isso seu objetivo"
    "agora e vender o maior numero de ovos sem atrasar e"
    "sem inutilizar o veiculo\n";

  Escrever(text, 10, 65, 1, WHITE, 0);
}