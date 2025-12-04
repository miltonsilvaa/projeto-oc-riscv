#include "teclado.h"
#include <iostream>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

Teclado::Teclado() : teclaPressionada(0), interrupcaoPendente(false) {
    std::cout << "⌨️ TECLADO: Dispositivo de entrada inicializado" << std::endl;
}

bool Teclado::verificarInterrupcao() {
#ifdef _WIN32
    if (_kbhit()) {
        teclaPressionada = _getch();
        interrupcaoPendente = true;
        std::cout << "⌨️ TECLADO: Tecla pressionada: '" << teclaPressionada << "'" << std::endl;
        return true;
    }
#else
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        teclaPressionada = ch;
        interrupcaoPendente = true;
        std::cout << "⌨️ TECLADO: Tecla pressionada: '" << (char)teclaPressionada << "'" << std::endl;
        return true;
    }
#endif
    
    return interrupcaoPendente;
}

uint8_t Teclado::lerTecla() {
    if (interrupcaoPendente) {
        interrupcaoPendente = false;
        return teclaPressionada;
    }
    return 0;
}

void Teclado::simularTecla(uint8_t tecla) {
    teclaPressionada = tecla;
    interrupcaoPendente = true;
    std::cout << "⌨️ TECLADO: Tecla simulada: '" << (char)tecla << "'" << std::endl;
}