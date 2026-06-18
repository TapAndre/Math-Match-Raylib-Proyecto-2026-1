//Carlos Andrée Mejía Tapia - Alexis Ponce Montaño 
//Programación Estructurada Grupo 432
//Proyecto Final: Match Match - Memorama de Multiplicaciones

#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define filas 5
#define columnas 12
#define cartasTotales 60

Vector2 posicionTitulo = {430, 130};

typedef struct Card {
    Rectangle rect;
    char text[20]; //reserva 20 bytes
    bool revelado, emparejado, resultado;
    int parID; //identifica parejas
    Color colorParJugador;
} Card;

typedef enum PantallaJuego {
       Titulo,
       Menu,
       Intermision,
       UnJugador,
       Multijugador,
       RondaBonus,
       Instrucciones,
       GameOver,
       GameOver2 
} PantallaJuego;

PantallaJuego pantallaActual = Titulo, modoSeleccionado;

Card cards[cartasTotales];
Color jugadores[4] = {RED, BLUE, GREEN, YELLOW};

int carta1 = -1, carta2 = -1, jugadorActual = 0, ganador = 0, pares = 0;
float timer = 0, tiempoJuego = 180.0f, tiempoTurno = 15.0f, tiempoIntermision = 3.0f;

bool comprobarPar = false, continuar = false, mostrarTodasLasCartas = false, tiempoFuera = false;

int puntuaciones[4] = {0}, aciertos = 0, puntosTurno = 0;
int highScore = 0;

bool vivo[4] = {false, false, false, false};
int vivosRestantes = 0;

bool MuerteSubita = false;
int MS_factorA = 0;
int MS_factorB = 0;
int MS_resultadoCorrecto = 0;
int jugadorRespondiendo = -1;
bool revelarResultadoMS = false;
float tiempoResponderMS = 5.0f;

void crearCartas()
{
    bool resultadoUsado[101] = {false}; //bools para comprobar que no se repita la operacion

    int index = 0;
    int par = 0;

    while(index < cartasTotales) //index es la posicion de la carta
    {
        int a, b, resultado;

        do
        {
            a = GetRandomValue(1, 10); //por cada index se genera un par con una mult y un resultado
            b = GetRandomValue(1, 10);

            resultado = a * b;

        } while(resultadoUsado[resultado]);

        resultadoUsado[resultado] = true;

        //operación
        sprintf(cards[index].text, "%d x %d", a, b);

        cards[index].revelado = false;
        cards[index].emparejado = false;
        cards[index].resultado = false;
        cards[index].parID = par;
        cards[index].colorParJugador = LIGHTGRAY;

        index++;

        //resultado
        sprintf(cards[index].text, "%d", resultado);

        cards[index].revelado = false;
        cards[index].emparejado = false;
        cards[index].resultado = true;
        cards[index].parID = par;
        cards[index].colorParJugador = LIGHTGRAY;

        index++;
        par++;
    }
}

void azarCartas()
{
    for(int i = 0; i < cartasTotales; i++)
    {
        int randomIndex =
        GetRandomValue(0, cartasTotales - 1);

        Card temp = cards[i];
        cards[i] = cards[randomIndex];
        cards[randomIndex] = temp;
    }

    int anchoCarta = 80; // tamaños
    int altoCarta = 100;

    int espacioX = 12; // espaciado
    int espacioY = 12;

    // Centrar tablero
    int anchoTablero = (columnas * anchoCarta) + ((columnas - 1) * espacioX);
    int inicioX = (1300 - anchoTablero) / 2;
    int inicioY = 90;
    // Reacomodar posiciones
    int index = 0;

    for(int fila = 0; fila < filas; fila++)
    {
        for(int col = 0; col < columnas; col++)
        {
            cards[index].rect.x =
            inicioX +
            col * (anchoCarta + espacioX);

            cards[index].rect.y =
            inicioY +
            fila * (altoCarta + espacioY);

            cards[index].rect.width = anchoCarta;
            cards[index].rect.height = altoCarta;

            index++;
        }
    }
}

void UpdateGame() //actualizar juego
{
    if(IsKeyPressed(KEY_H)) //debug
    {
        mostrarTodasLasCartas = !mostrarTodasLasCartas; //cambiar la bandera cada vez que se presiona H
    }

    Vector2 mouse = GetMousePosition();

    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !comprobarPar && !continuar)
    {
        for(int i = 0; i < cartasTotales; i++)
        {
            if(CheckCollisionPointRec(mouse, cards[i].rect)) //detectar click
            {

                // Evitar seleccionar reveladas
                if(!cards[i].revelado && !cards[i].emparejado)
                {
                    cards[i].revelado = true;
                    // 1ra carta
                    if(carta1 == -1)
                    {
                        carta1 = i;
                    }
                    // 2da carta
                    else if(carta2 == -1)
                    {
                        carta2 = i;
                        comprobarPar = true;
                    }
                }
            }
        }
    }

    if(comprobarPar)
    {
        timer += GetFrameTime();

        // Esperar medio segundo para no afectar el tiempo del jugador
        if(timer >= 0.5f)
        {
            // Si coinciden
            if(cards[carta1].parID == cards[carta2].parID && cards[carta1].resultado != cards[carta2].resultado)
            {
                cards[carta1].emparejado = true;
                cards[carta2].emparejado = true;
                
                aciertos++;
                puntosTurno += 10;
                pares++;

                if(pares >= 30)
                {
                    if(pantallaActual == Multijugador)
                    {
                        puntuaciones[jugadorActual] += puntosTurno;
                        puntosTurno = 0; //asegurar que la puntuacion se actualice antes de terminar
                        
                        pantallaActual = GameOver2;
                    }
                    else
                    {
                    pantallaActual = GameOver;
                    }

                    return; // salir de UpdateGame
                }

                if(pantallaActual == Multijugador){
                    continuar = true;
                }
                
                cards[carta1].colorParJugador = jugadores[jugadorActual];
                cards[carta2].colorParJugador = jugadores[jugadorActual];
            }
            else
            {
                // Ocultar si fallan
                cards[carta1].revelado = false;
                cards[carta2].revelado = false;

                puntosTurno = 0;
                tiempoJuego = tiempoJuego - 5;
                jugadorActual++;

                if(jugadorActual > 3)
                {
                jugadorActual = 0;
                }
                tiempoTurno = 15.0f; //reinicia tiempo cuando falla el par
            }

            // Reiniciar variables
            carta1 = -1;
            carta2 = -1;
            timer = 0;
            comprobarPar = false;
        }
    }

    if(pantallaActual == UnJugador)
    {
        continuar = false;
    }

    if(continuar && pantallaActual == Multijugador)
    {

        if(IsKeyPressed(KEY_ENTER)) // Turno extra con menos tiempo
        {
            continuar = false;
            tiempoTurno = 10.0f;
        }

        if(IsKeyPressed(KEY_SPACE)) // Sig. jugador
        {
            puntuaciones[jugadorActual] += puntosTurno;
            puntosTurno = 0;
            continuar = false;
            jugadorActual++;

            if(jugadorActual > 3)
            {
            jugadorActual = 0;
            }
            tiempoTurno = 15.0f; //reinicia el tiempo al cambiar de jugador
        }
    }
}

void DrawGame() //dibujar cartas
{
    Vector2 mouse = GetMousePosition();

    for(int i = 0; i < cartasTotales; i++)
    {
        if(cards[i].revelado || cards[i].resultado || mostrarTodasLasCartas) // carta revelada
        {
            if(cards[i].emparejado)
            {
            DrawRectangleRec(cards[i].rect, cards[i].colorParJugador);
            }
        else
        {
        DrawRectangleRec(cards[i].rect,RAYWHITE);
        }
            DrawText(cards[i].text, cards[i].rect.x + 15, cards[i].rect.y + 45, 20, DARKBLUE);
            // text, posX, posY, tamañoFuente, color
        }
        else
        {
            DrawRectangleRec(cards[i].rect, SKYBLUE); // carta oculta
            DrawText("?", cards[i].rect.x + 38, cards[i].rect.y + 35, 40, WHITE);
        }
        if(CheckCollisionPointRec(mouse, cards[i].rect) && !cards[i].revelado && !cards[i].emparejado)
        {
            DrawRectangleLinesEx(cards[i].rect, 4, YELLOW);

            // resaltado amarillo
            DrawRectangleRec(cards[i].rect, Fade(YELLOW, 0.20f)
            );
        }
        else
        {
            DrawRectangleLinesEx(cards[i].rect, 2, DARKBLUE);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////// MAIN

int main()
{
    const int ancho = 1300, alto = 700;
    InitWindow(ancho, alto,"Memorama de Multiplicaciones");
    Image iconoVentana = LoadImage("C:/Estructurada/MemoramaMultiplicaciones/recursos/iconoVentana.png");
    SetWindowIcon(iconoVentana);

    Font font = LoadFont("C:/Estructurada/MemoramaMultiplicaciones/recursos/jupiter_crash.png");
    //Fuente obtenida de https://github.com/raysan5/raylib
    SetTargetFPS(60);

    //////////////////////////////////////////////////////////////////////////////////// IMAGENES

    Texture2D fondoJuego, fondoTitulo, fondoMenu, fondoInstrucciones, fondoGameOver, fondoReady;
    fondoJuego = LoadTexture("C:/Estructurada/MemoramaMultiplicaciones/recursos/fondojuego.png");
    fondoTitulo = LoadTexture("C:/Estructurada/MemoramaMultiplicaciones/recursos/fondoTitulo.png");
    fondoMenu = LoadTexture("C:/Estructurada/MemoramaMultiplicaciones/recursos/fondoMenu.png");
    fondoInstrucciones = LoadTexture("C:/Estructurada/MemoramaMultiplicaciones/recursos/fondoInstrucciones.png");
    fondoGameOver = LoadTexture("C:/Estructurada/MemoramaMultiplicaciones/recursos/fondoGameOver.png");
    fondoReady = LoadTexture("C:/Estructurada/MemoramaMultiplicaciones/recursos/fondoReady.png");
    
    //////////////////////////////////////////////////////////////////////////////////// IMAGENES

    crearCartas();
    azarCartas();

    FILE *archivo;
    archivo = fopen("highscore.dat", "rb");

if(archivo != NULL)
    {
        fread(&highScore, sizeof(int), 1, archivo);
        fclose(archivo);
    }

    bool evaluarGanadorNormal = true;

    while (!WindowShouldClose())
    {
    float hue = fmod(GetTime() * 100, 360);
    Color colorArcoiris = ColorFromHSV(hue, 1.0f, 1.0f);
    Color colorArcoirisInvertido = ColorFromHSV(360 - hue, 1.0f, 1.0f);

    if(pantallaActual == UnJugador || pantallaActual == Multijugador)
    {
        UpdateGame();
    }
    
    BeginDrawing();
    ClearBackground(GRAY);
    
    if(pantallaActual == Titulo)
    {
        Rectangle source = {0, 0, fondoTitulo.width, fondoTitulo.height};
        Rectangle dest = {0, 0, ancho, alto};
        Rectangle botonJugar = {550, 280, 200, 60};
        Vector2 origin = {0,0}, mouse = GetMousePosition();

        DrawTexturePro(fondoTitulo, source, dest, origin, 0, WHITE);

        DrawTextEx(font, "MEMORAMA DE MULTIPLICACIONES", posicionTitulo, 40, 3, WHITE);

        DrawRectangleRec(botonJugar, DARKBLUE);
        DrawRectangleLinesEx(botonJugar, 2, WHITE);
        DrawText("JUGAR", 600, 295, 30, colorArcoiris);

        if(CheckCollisionPointRec(mouse, botonJugar))
        {
            DrawRectangleLinesEx(botonJugar, 4, YELLOW);
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                pantallaActual = Menu;
            }
        }

        DrawText("Carlos Andree Mejia Tapia \nAlexis Ponce Montaño", 960, 650, 20, BLACK);
    }
    
    else if(pantallaActual == Menu)
    {
        Rectangle source = {0, 0, fondoMenu.width, fondoMenu.height};
        Rectangle dest = {0, 0, ancho, alto};
        Vector2 origin = {0,0};
        ClearBackground(BEIGE);
        DrawTexturePro(fondoMenu, source, dest, origin, 0, WHITE);
        DrawText("Selecciona una opción:", 360, 15, 50, colorArcoiris);
        DrawText("1 - UN JUGADOR", 550, 500, 20, WHITE);
        DrawText("2 - MULTIJUGADOR", 550, 540, 20, WHITE);
        DrawText("3 - INSTRUCCIONES", 550, 580, 20, WHITE);

        DrawText("ESC - SALIR     M - PANTALLA DE TÍTULO", 360, 650, 30, WHITE);

        if(IsKeyPressed(KEY_ONE))
        {
            pantallaActual = Intermision;
            tiempoIntermision = 3.0f;
            modoSeleccionado = UnJugador;

            tiempoJuego = 180.0f;
            jugadorActual = 0;
            pares = 0;
            aciertos = 0;
            crearCartas();
            azarCartas();
        }

        if(IsKeyPressed(KEY_TWO))
        {
            pantallaActual = Intermision;
            tiempoIntermision = 3.0f;
            modoSeleccionado = Multijugador;

            ganador = 0;
            pares = 0;
            jugadorActual = 0;
            tiempoTurno = 15.0f;
            puntuaciones[0] = 0;
            puntuaciones[1] = 0;
            puntuaciones[2] = 0;
            puntuaciones[3] = 0;

            MuerteSubita = false;
            jugadorRespondiendo = -1;
            revelarResultadoMS = false;
            evaluarGanadorNormal = true;

            crearCartas();
            azarCartas();
        }

        if(IsKeyPressed(KEY_THREE))
        {
            pantallaActual = Instrucciones;
        }

        if(IsKeyPressed(KEY_ESCAPE))
        {
            break;
        }

        if(IsKeyPressed(KEY_M))
        {
            pantallaActual = Titulo;
        }
    }

    else if(pantallaActual == UnJugador)
    {
        tiempoJuego -= GetFrameTime();
        if(tiempoJuego <= 0)
        {
            tiempoJuego = 0;
            pantallaActual = GameOver;
        }

        if(IsKeyPressed(KEY_P))
        {
            pares = 29;
        }
        
        if(IsKeyPressed(KEY_S))
        {
            tiempoJuego = tiempoJuego - 50;
        }

        Rectangle source = {0, 0, fondoJuego.width, fondoJuego.height};
        Rectangle dest = {0, 0, ancho, alto};
        Vector2 origin = {0,0};
        DrawTexturePro(fondoJuego, source, dest, origin, 0, WHITE);
        //DrawTextEx(font, "MEMORAMA DE MULTIPLICACIONES", posicion, 40, 3, WHITE);
        DrawRectangle(558, 30, 180, 50, BLACK);
        DrawText(TextFormat("Tiempo: %.0f", tiempoJuego), 565, 40, 30, RED);
        DrawGame();
        DrawRectangle(10, 650, 140, 40, BLACK);
        DrawText("M --> MENU", 20, 660, 20, colorArcoiris);

        if(IsKeyPressed(KEY_M))
        {
            pantallaActual = Menu;
        }

        if(IsKeyPressed(KEY_X)) //debug
        {
            pantallaActual = GameOver;
        }
    }

    else if(pantallaActual == Multijugador)
    {
        if(!continuar)
        {
            tiempoTurno -= GetFrameTime(); //el tiempo está corriendo a menos que le pregunte al jugador sobre el turno
        }

        if(tiempoTurno <= 0)
        {
            if(carta1 != -1)
            {
                cards[carta1].revelado = false;
            }

            if(carta2 != -1)
            {
                cards[carta2].revelado = false;
            }

            carta1 = -1;
            carta2 = -1;
            puntosTurno = 0;
            continuar = false;
            jugadorActual++;

            if(jugadorActual > 3)
            {
                jugadorActual = 0;
            }
            tiempoTurno = 15.0f;
        }
        
        Rectangle source = {0, 0, fondoJuego.width, fondoJuego.height};
        Rectangle dest = {0, 0, ancho, alto};
        Vector2 origin = {0,0};
        DrawTexturePro(fondoJuego, source, dest, origin, 0, WHITE);
        //DrawTextEx(font, "MEMORAMA DE MULTIPLICACIONES", posicion, 40, 3, WHITE);
        DrawRectangle(558, 30, 180, 50, BLACK);
        DrawText(TextFormat("Turno J%d", jugadorActual + 1), 565, 40, 30, jugadores[jugadorActual]);
        DrawText(TextFormat("Tiempo: %.0f", tiempoTurno), 850, 40, 25, jugadores[jugadorActual]);
        DrawGame();

        if(continuar)
        {
                DrawRectangle(225, 250, 850, 150, BLACK);
                DrawText("Pareja correcta!", 470, 270, 35, GREEN);
                DrawText("ENTER --> Turno extra", 430, 330, 25, WHITE);
                DrawText("ESPACIO --> Guardar puntos y continuar al sig. jugador", 300, 370, 25, YELLOW);
        }

        DrawText(TextFormat("J1: %d", puntuaciones[0]), 20, 20, 25, RED);
        DrawText(TextFormat("J2: %d", puntuaciones[1]), 20, 50, 25, BLUE);
        DrawText(TextFormat("J3: %d", puntuaciones[2]), 20, 80, 25, GREEN);
        DrawText(TextFormat("J4: %d", puntuaciones[3]), 20, 110, 25, YELLOW);
        DrawRectangle(10, 650, 140, 40, BLACK);
        DrawText("M --> MENU", 20, 660, 20, colorArcoiris);

        if(IsKeyPressed(KEY_M))
        {
            pantallaActual = Menu;
        }

        if(IsKeyPressed(KEY_P)) //contador de pares para terminar el juego por todos los pares revelados
        {
            pares = 29;
        }

        if(IsKeyPressed(KEY_X)) //debug
        {
            pantallaActual = GameOver2;
        }
    }


    else if(pantallaActual == Intermision)
    {
            Rectangle source = {0, 0, fondoReady.width, fondoReady.height};
            Rectangle dest = {0, 0, ancho, alto};
            Vector2 origin = {0,0};
            DrawTexturePro(fondoReady, source, dest, origin, 0, WHITE);

            tiempoIntermision -= GetFrameTime();

            ClearBackground(BLACK);
            DrawText(TextFormat("%.0f", tiempoIntermision), 620, 380, 80, RED);

            if(tiempoIntermision <= 0)
            {
                pantallaActual = modoSeleccionado;
            }

            if(IsKeyPressed(KEY_M))
            {
                pantallaActual = Menu;
            }
    }


    else if(pantallaActual == Instrucciones)
    {
        Rectangle source = {0, 0, fondoInstrucciones.width, fondoInstrucciones.height};
        Rectangle dest = {0, 0, ancho, alto};
        Vector2 origin = {0,0};
        DrawTexturePro(fondoInstrucciones, source, dest, origin, 0, WHITE);
        ClearBackground(WHITE);
        DrawText("M --> MENU", 20, 660, 20, colorArcoiris);
        if(IsKeyPressed(KEY_M))
        {
            pantallaActual = Menu;
        }
    }
    
    else if(pantallaActual == GameOver)
    {
        Rectangle source = {0, 0, fondoGameOver.width, fondoGameOver.height};
        Rectangle dest = {0, 0, ancho, alto};
        Vector2 origin = {0,0};
        DrawTexturePro(fondoGameOver, source, dest, origin, 0, WHITE);

        if(tiempoJuego <= 0)
{
            if(aciertos > highScore)
            {
                highScore = aciertos;

                archivo = fopen("highscore.dat", "wb");
                if(archivo)
                {
                    fwrite(&highScore, sizeof(int), 1, archivo);
                    fclose(archivo);
                }
        }

    pantallaActual = GameOver;
}
        ClearBackground(BLACK);
        DrawText("Tiempo fuera!", 480, 15, 50, colorArcoiris);
        DrawText(TextFormat("Pares revelados:"), 280, 300, 50, RED);
        DrawText(TextFormat("%d", aciertos), 950, 250, 150, colorArcoiris);
        DrawText(TextFormat("High Score: %d", highScore), 825, 450, 50, colorArcoirisInvertido);
        DrawText("R ----> Borrar High Score" , 825, 550, 25, RED);
        DrawText("Gracias por jugar. \nM --> MENU", 500, 600, 30, colorArcoiris);
        
        if(IsKeyPressed(KEY_M))
        {
            pantallaActual = Menu;
        }

        if(IsKeyPressed(KEY_R))
        {
            highScore = 0;
            archivo = fopen("highscore.dat", "wb");

            if(archivo)
            {
                fwrite(&highScore, sizeof(int), 1, archivo);
                fclose(archivo);
            }  
        }
    }

    else if(pantallaActual == GameOver2)
    {
        if (evaluarGanadorNormal)
        {
            int maxPuntos = puntuaciones[0];
            for(int i = 1; i < 4; i++) 
            {
                if(puntuaciones[i] > maxPuntos) 
                {
                    maxPuntos = puntuaciones[i];
                }
            }

            vivosRestantes = 0; 
            for(int i = 0; i < 4; i++) 
            {
                if(puntuaciones[i] == maxPuntos) 
                {
                    vivo[i] = true; 
                    vivosRestantes++;
                } 
                else 
                {
                    vivo[i] = false;
                }
            }

            if (vivosRestantes > 1) 
            {
                MuerteSubita = true;
                MS_factorA = GetRandomValue(6, 9); 
                MS_factorB = GetRandomValue(6, 9);
                MS_resultadoCorrecto = MS_factorA * MS_factorB;
                jugadorRespondiendo = -1;
                revelarResultadoMS = false;
                tiempoResponderMS = 5.0f; // Inicializar temporizador
            }
            else 
            {
                MuerteSubita = false;
                ganador = 0;
                for(int i = 1; i < 4; i++) 
                {
                    if(puntuaciones[i] > puntuaciones[ganador])
                    {
                        ganador = i;
                    }
                }
            }
            evaluarGanadorNormal = false; 
        }

        if (MuerteSubita)
        {
            if (jugadorRespondiendo == -1)
            {
                if (IsKeyPressed(KEY_ONE) && vivo[0]) jugadorRespondiendo = 0;
                if (IsKeyPressed(KEY_TWO) && vivo[1]) jugadorRespondiendo = 1;
                if (IsKeyPressed(KEY_THREE) && vivo[2]) jugadorRespondiendo = 2;
                if (IsKeyPressed(KEY_FOUR) && vivo[3]) jugadorRespondiendo = 3;
            }
            else if (!revelarResultadoMS)
            {
                // Restar tiempo real si seleccionó su botón pero no ha presionado ENTER
                tiempoResponderMS -= GetFrameTime();

                if (tiempoResponderMS <= 0.0f)
                {
                    vivo[jugadorRespondiendo] = false; 
                    vivosRestantes--;

                    if (vivosRestantes <= 1)
                    {
                        for(int i = 0; i < 4; i++) 
                        {
                            if(vivo[i]) { ganador = i; break; }
                        }
                        MuerteSubita = false;
                    }
                    else
                    {
                        MS_factorA = GetRandomValue(6, 9);
                        MS_factorB = GetRandomValue(6, 9);
                        MS_resultadoCorrecto = MS_factorA * MS_factorB;
                        jugadorRespondiendo = -1;
                        revelarResultadoMS = false;
                        tiempoResponderMS = 5.0f; // Reset tiempo para el siguiente
                    }
                }

                if (IsKeyPressed(KEY_ENTER))
                {
                    revelarResultadoMS = true;
                    ganador = jugadorRespondiendo; 
                }
            }
            else 
            {
                if (IsKeyPressed(KEY_UP)) 
                {
                    MuerteSubita = false; 
                }
                if (IsKeyPressed(KEY_DOWN)) 
                {
                    vivo[jugadorRespondiendo] = false; 
                    vivosRestantes--;

                    if (vivosRestantes <= 1)
                    {
                        for(int i = 0; i < 4; i++) 
                        {
                            if(vivo[i]) { ganador = i; break; }
                        }
                        MuerteSubita = false;
                    }
                    else
                    {
                        MS_factorA = GetRandomValue(6, 9);
                        MS_factorB = GetRandomValue(6, 9);
                        MS_resultadoCorrecto = MS_factorA * MS_factorB;
                        jugadorRespondiendo = -1;
                        revelarResultadoMS = false;
                        tiempoResponderMS = 10.0f; // Reset tiempo
                    }
                }
            }
        }

        // --- RENDERIZADO DE PANTALLA ---
        Rectangle source = {0, 0, fondoGameOver.width, fondoGameOver.height};
        Rectangle dest = {0, 0, ancho, alto};
        Vector2 origin = {0,0};
        DrawTexturePro(fondoGameOver, source, dest, origin, 0, WHITE);
        ClearBackground(BLACK);

        if (MuerteSubita)
        {
            DrawRectangle(250, 80, 800, 480, DARKGRAY);
            DrawText("¡DESEMPATE DE MUERTE SÚBITA!", 350, 110, 35, RED);
            DrawText("Resuelvan la siguiente operación mentalmente:", 320, 170, 24, WHITE);
                
            DrawText(TextFormat("%d x %d = ?", MS_factorA, MS_factorB), 520, 230, 60, colorArcoiris);

            if (jugadorRespondiendo == -1)
            {
                DrawText("¡El primero que sepa la respuesta presione su TECLA DE JUGADOR!", 280, 340, 22, YELLOW);
                    
                char textoEmpatados[100] = "Solo pueden responder: ";
                if (vivo[0]) strcat(textoEmpatados, "J1 ");
                if (vivo[1]) strcat(textoEmpatados, "J2 ");
                if (vivo[2]) strcat(textoEmpatados, "J3 ");
                if (vivo[3]) strcat(textoEmpatados, "J4 ");
                DrawText(textoEmpatados, 380, 390, 22, LIGHTGRAY);
            }
            else 
            {
                DrawRectangle(300, 330, 700, 200, BLACK);
                DrawText(TextFormat("JUGADOR %d RESPONDE", jugadorRespondiendo + 1), 480, 350, 26, jugadores[jugadorRespondiendo]);
                    
                if (!revelarResultadoMS)
                {
                    DrawText("Dila en voz alta y presiona ENTER para verificar", 360, 390, 22, WHITE);
                    DrawText(TextFormat("TIEMPO RESTANTE: %.1f seg", tiempoResponderMS), 450, 440, 26, RED);
                }
                else 
                {
                    DrawText(TextFormat("Resultado Real: %d", MS_resultadoCorrecto), 480, 400, 30, GREEN);
                    DrawText("¿Contestó Correctamente?", 480, 450, 22, WHITE);
                    DrawText("[FLECHA ARRIBA] = SÍ (Gana)  |  [FLECHA ABAJO] = NO (Eliminado)", 290, 490, 22, colorArcoirisInvertido);
                }
            }
        }
        else 
        {
            DrawText("Tabla de puntuación:", 370, 15, 50, colorArcoiris);
            DrawText(TextFormat("Jugador 1: %d", puntuaciones[0]), 280, 300, 40, RED);
            DrawText(TextFormat("Jugador 2: %d", puntuaciones[1]), 280, 350, 40, BLUE);
            DrawText(TextFormat("Jugador 3: %d", puntuaciones[2]), 280, 400, 40, GREEN);
            DrawText(TextFormat("Jugador 4: %d", puntuaciones[3]), 280, 450, 40, YELLOW);
                
            DrawText(TextFormat("J%d", ganador + 1), 950, 250, 150, colorArcoiris);
            DrawText("¡Ganaste!", 825, 450, 50, colorArcoirisInvertido);
            DrawText("Gracias por jugar. \nM --> MENU", 500, 600, 30, colorArcoiris);
        }
            
        if(IsKeyPressed(KEY_M) && !MuerteSubita)
        {
            evaluarGanadorNormal = true;
            modoSeleccionado = Menu;
            pantallaActual = Menu;
        }
    }

    EndDrawing();
}

    //liberar memoria VRAM
    UnloadTexture(fondoTitulo);
    UnloadTexture(fondoMenu);
    UnloadTexture(fondoJuego);
    UnloadTexture(fondoInstrucciones);
    UnloadTexture(fondoReady);
    UnloadImage(iconoVentana);
    UnloadTexture(fondoGameOver);
    CloseWindow();

    return 0;
}