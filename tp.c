
// bibliotecas que eu add
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <allegro5/allegro_audio.h>   
#include <allegro5/allegro_acodec.h> 

//bibliotecas que já estavam
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

const float FPS = 60;
const int SCREEN_W = 960;
const int SCREEN_H = 540;
const int GRASS_H = 60;

const int NAVE_W = 73;
const int NAVE_H = 100;

const int ALIEN_W = 90;
const int ALIEN_H = 79;

const int ESP_X = 19;
const int ESP_Y = 15;

const int ALIEN_DISPLAY_W = 50;
const int ALIEN_DISPLAY_H = 44;

const int TIRO_W = 10;
const int TIRO_H = 17;

//-------------------------------------------------------PAGINAS DO JOGO---------------------------------------------------

typedef enum {
    MENU,
    JOGANDO,
    FIM_DE_JOGO
} EstadoJogo;

//-------------------------------------------------------NAVE E CENARIO --------------------------------------------------------------
typedef struct Nave {
    float x;
    float y;
    float velocidade;
    int dir, esq;
    ALLEGRO_BITMAP *sprite_da_nave;
} Nave;

void initNave(Nave *nave) {
    nave->x = SCREEN_W/2 - NAVE_W/2;
    nave->velocidade = 2.5;
    nave->dir = 0;
    nave->esq = 0;
    nave->y = SCREEN_H - GRASS_H - NAVE_H;
    
    nave->sprite_da_nave = al_load_bitmap("navesprite.png");
    if(!nave->sprite_da_nave) {
        fprintf(stderr, "Falha ao carregar o sprite da nave!\n");
        exit(1);
    }
}


void draw_scenario(ALLEGRO_BITMAP *fundo) {
    al_draw_bitmap(fundo, 0, 0, 0);
}

void draw_nave(Nave nave) {
    al_draw_bitmap(nave.sprite_da_nave, nave.x, nave.y, 0);
}

void update_nave(Nave *nave) {
    if(nave->dir && nave->x + nave->velocidade <= SCREEN_W - 90) {
        nave->x += nave->velocidade;
    }
    
    if(nave->esq && nave->x - nave->velocidade >= 0) {
        nave->x -= nave->velocidade;
    }
}

//---------------------------------------------------------HORDA DOS ALIENS-----------------------------------------------

typedef struct Alien {
    float x, y;
    bool vivo;
    int sprite_coluna;
    int sprite_linha;
} Alien;

void init_horda(Alien inimigos[4][5]) {
    int i, j;
    int esp_x = ALIEN_DISPLAY_W + 30;
    int esp_y = ALIEN_DISPLAY_H + 20;
    int margens_x = 50;
    int margens_y = 40;

    for(i = 0; i < 4; i++) {
        for(j = 0; j < 5; j++) {
            Alien* alien_atual = &inimigos[i][j];
            alien_atual->vivo = true;
            alien_atual->x = margens_x + j * esp_x;
            alien_atual->y = margens_y + i * esp_y;
            alien_atual->sprite_linha = rand() % 2;
            alien_atual->sprite_coluna = rand() % 3;
        }
    }
}

void draw_horda(Alien inimigos[4][5], ALLEGRO_BITMAP *spritesheet) {
    int i, j;
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 5; j++) {
            if(inimigos[i][j].vivo) {
                float sprite_x = inimigos[i][j].sprite_coluna * (ALIEN_W + ESP_X);
                float sprite_y = inimigos[i][j].sprite_linha * (ALIEN_H + ESP_Y);

                al_draw_scaled_bitmap(spritesheet,
                                    sprite_x, sprite_y,
                                    ALIEN_W, ALIEN_H,
                                    inimigos[i][j].x, inimigos[i][j].y,
                                    ALIEN_DISPLAY_W, ALIEN_DISPLAY_H,
                                    0);
            }
        }
    }
}

void update_horda(Alien inimigos[4][5], float *vel_horda_x) {
    int i, j;
    bool bateu_borda = false;

    for(i = 0; i < 4; i++) {
        for(j = 0; j < 5; j++) {
            if(inimigos[i][j].vivo) {
                inimigos[i][j].x += (*vel_horda_x);
            }
        }
    }

    for(i = 0; i < 4; i++) {
        for(j = 0; j < 5; j++) {
            if(inimigos[i][j].vivo) {
                if((inimigos[i][j].x + ALIEN_DISPLAY_W > SCREEN_W) || (inimigos[i][j].x < 0)) {
                    bateu_borda = true;
                    break;
                }
            }
        }
        if(bateu_borda) break;
    }

    if(bateu_borda) {
        *vel_horda_x *= -1;
        for(i = 0; i < 4; i++) {
            for(j = 0; j < 5; j++) {
                if(inimigos[i][j].vivo) {
                    inimigos[i][j].y += ALIEN_DISPLAY_H/2;
                }
            }
        }
    }
}

//---------------------------------------------------------------COLISAO COM O CHAO-----------------------------------------------------

bool verifica_colisao_horda_solo(Alien inimigos[4][5]) {
    int i, j;
    int topo_do_chao = SCREEN_H - 130;

    for(i = 0; i < 4; i++) {
        for(j = 0; j < 5; j++) {
            if(inimigos[i][j].vivo && (inimigos[i][j].y + ALIEN_DISPLAY_H > topo_do_chao)) {
                return true;
            }
        }
    }
    return false;
}

//----------------------------------------------------------------POW POW (TIRO :0)-----------------------------------------------------------

typedef struct Tiro {
    float x, y;
    float vel;
    bool ativo;
    bool explodindo;
    int tempo_explosao;
    ALLEGRO_BITMAP *explosao_sprite;
    ALLEGRO_BITMAP *sprite_bala;
} Tiro;

void init_tiro(Tiro *tiro, ALLEGRO_BITMAP *sprite_bala, ALLEGRO_BITMAP *explosao_sprite) {
    tiro->ativo = false;
    tiro->explodindo = false;
    tiro->vel = -6;
    tiro->tempo_explosao = 0;
    tiro->explosao_sprite = explosao_sprite;
    tiro->sprite_bala = sprite_bala;
}

void update_tiro(Tiro *tiro) {
    if(tiro->ativo) {
        tiro->y += tiro->vel;
        if(tiro->y < 0) {
            tiro->ativo = false;
        }
    }

    if(tiro->explodindo) {
        tiro->tempo_explosao--;
        if(tiro->tempo_explosao <= 0) {
            tiro->explodindo = false;
        }
    }
}

void draw_tiro(Tiro tiro) {
    if(tiro.ativo && tiro.sprite_bala) {
        al_draw_bitmap(tiro.sprite_bala, tiro.x, tiro.y, 0);
    }
    
    if(tiro.explodindo && tiro.explosao_sprite) {
        al_draw_bitmap(tiro.explosao_sprite, tiro.x - 17, tiro.y - 17, 0);
    }
}

//---------------------------------------------------------------COLISÃO TIRO COM ALIEN ---------------------------------------------------

bool verifica_colisao_tiro_alien(Tiro *tiro, Alien inimigos[4][5]) {
    if(!tiro->ativo) return false;
    int i, j;

    for(i = 0; i < 4; i++) {
        for(j = 0; j < 5; j++) {
            if(inimigos[i][j].vivo) {
                if(tiro->x < inimigos[i][j].x + ALIEN_DISPLAY_W &&
                   tiro->x + TIRO_W > inimigos[i][j].x &&
                   tiro->y < inimigos[i][j].y + ALIEN_DISPLAY_H &&
                   tiro->y + TIRO_H > inimigos[i][j].y) {
                    
                    inimigos[i][j].vivo = false;
                    tiro->ativo = false;
                    tiro->explodindo = true;
                    tiro->tempo_explosao = 30;
                    return true;
                }
            }
        }
    }
    return false;
}

//----------------------------------------------------------------COLISÃO ALIENS COM NAVE-----------------------------------------------------

bool verifica_colisao_alien_nave(Alien inimigos[4][5], Nave *nave) {
    int i, j;
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 5; j++) {
            if(inimigos[i][j].vivo) {
                if(inimigos[i][j].x < nave->x + NAVE_W &&
                   inimigos[i][j].x + ALIEN_DISPLAY_W > nave->x &&
                   inimigos[i][j].y < nave->y + NAVE_H &&
                   inimigos[i][j].y + ALIEN_DISPLAY_H > nave->y) {
                    return true;
                }
            }
        }
    }
    return false;
}

//----------------------------------------------------------------PONTUAÇÃO-------------------------------------------------

typedef struct {
    int pontuacao_atual;
    int recorde;
} Pontuacao;

void init_pontuacao(Pontuacao *pont) {
    pont->pontuacao_atual = 0;
    FILE *arquivo = fopen("recorde.txt", "r");
    if(arquivo) {
        fscanf(arquivo, "%d", &pont->recorde);
        fclose(arquivo);
    } else {
        pont->recorde = 0;
    }
}

void update_recorde(Pontuacao *pont) {
    if(pont->pontuacao_atual > pont->recorde) {
        pont->recorde = pont->pontuacao_atual;
        FILE *arquivo = fopen("recorde.txt", "w");
        if(arquivo) {
            fprintf(arquivo, "%d", pont->recorde);
            fclose(arquivo);
        }
    }
}

void draw_pontuacao(Pontuacao pont, ALLEGRO_FONT *font) {
    if(font) {
        // Sombra para RECORD
        al_draw_textf(font, al_map_rgb(0, 0, 0), SCREEN_W - 9, 9, ALLEGRO_ALIGN_RIGHT, "RECORD: %d", pont.recorde);
        al_draw_textf(font, al_map_rgb(0, 0, 0), SCREEN_W - 11, 9, ALLEGRO_ALIGN_RIGHT, "RECORD: %d", pont.recorde);
        al_draw_textf(font, al_map_rgb(0, 0, 0), SCREEN_W - 9, 11, ALLEGRO_ALIGN_RIGHT, "RECORD: %d", pont.recorde);
        al_draw_textf(font, al_map_rgb(0, 0, 0), SCREEN_W - 11, 11, ALLEGRO_ALIGN_RIGHT, "RECORD: %d", pont.recorde);
        
        // Texto principal
        al_draw_textf(font, al_map_rgb(41, 74, 183), 10, 10, 0, "SCORE: %d", pont.pontuacao_atual);
        al_draw_textf(font, al_map_rgb(41, 74, 183), SCREEN_W - 10, 10, ALLEGRO_ALIGN_RIGHT, "RECORD: %d", pont.recorde);
    }
}

//----------------------------------------------------------------TELAS DO JOGO-------------------------------------------------

void draw_menu(ALLEGRO_BITMAP *tela_menu, ALLEGRO_FONT *font) {
    al_draw_bitmap(tela_menu, 0, 0, 0);
    al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H - 100, 
                 ALLEGRO_ALIGN_CENTER, "Pressione ENTER para comecar");
}

void draw_fim_de_jogo(ALLEGRO_BITMAP *tela_fim_de_jogo, Pontuacao pontuacao, ALLEGRO_FONT *font) {
    al_draw_bitmap(tela_fim_de_jogo, 0, 0, 0);
    
    // Título
    al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2 - 60, 
                 ALLEGRO_ALIGN_CENTER, "GAME OVER");
    
    // Pontuação final
    al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2 - 20, 
                  ALLEGRO_ALIGN_CENTER, "SCORE: %d", pontuacao.pontuacao_atual);
    
    // Recorde
    al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2 + 20, 
                  ALLEGRO_ALIGN_CENTER, "RECORD: %d", pontuacao.recorde);
    
    // Instruções
    al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2 + 80, 
                 ALLEGRO_ALIGN_CENTER, "Pressione R para jogar novamente");
    al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2 + 110, 
                 ALLEGRO_ALIGN_CENTER, "Pressione ESC para sair");
}

//---------------------------------------------------------------- VERIFIQUE MORTE :P -------------------------------------------------

bool todos_aliens_mortos(Alien inimigos[4][5]) {
    int i, j;
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 5; j++) {
            if(inimigos[i][j].vivo) {
                return false;
            }
        }
    }
    return true;
}

//---------------------------------------------------------------MAIN-------------------------------------------------------------------------

int main(int argc, char **argv) {
    
    EstadoJogo estado_atual = MENU;
    
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_FONT *font = NULL;

    ALLEGRO_BITMAP *fundo = NULL;
    ALLEGRO_BITMAP *chao_sprite = NULL;
    ALLEGRO_BITMAP *alien_spritesheet = NULL;
    ALLEGRO_BITMAP *explosao_sprite = NULL;
    ALLEGRO_BITMAP *sprite_bala = NULL;
    ALLEGRO_BITMAP *tela_menu = NULL;
    ALLEGRO_BITMAP *tela_fim_de_jogo = NULL;
	ALLEGRO_SAMPLE *som_tiro = NULL;
    ALLEGRO_SAMPLE *som_explosao = NULL;
    ALLEGRO_AUDIO_STREAM *musica_fundo = NULL;
    ALLEGRO_BITMAP *sprite_da_nave = NULL;
    
    float horda_vel_x;
    
    srand(time(NULL));

    // Inicialização do Allegro
    if(!al_init()) {
        fprintf(stderr, "failed to initialize allegro!\n");
        return -1;
    }
    
    if(!al_init_primitives_addon()) {
        fprintf(stderr, "failed to initialize primitives!\n");
        return -1;
    }
    
    if(!al_init_image_addon()) {
        fprintf(stderr, "failed to initialize image module!\n");
        return -1;
    }

    al_init_font_addon();
    
    if(!al_init_ttf_addon()) {
        fprintf(stderr, "failed to load tff font module!\n");
        return -1;
    }
    
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
        fprintf(stderr, "failed to create timer!\n");
        return -1;
    }
    
    display = al_create_display(SCREEN_W, SCREEN_H);
    if(!display) {
        fprintf(stderr, "failed to create display!\n");
        al_destroy_timer(timer);
        return -1;
    }

    font = al_load_font("arial.ttf", 24, 1);
    if(font == NULL) {
        fprintf(stderr, "font file does not exist or cannot be accessed!\n");
    }

    event_queue = al_create_event_queue();
    if(!event_queue) {
        fprintf(stderr, "failed to create event_queue!\n");
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }
    
    if(!al_install_keyboard()) {
        fprintf(stderr, "failed to install keyboard!\n");
        return -1;
    }
    
    if(!al_install_mouse()) {
        fprintf(stderr, "failed to initialize mouse!\n");
        return -1;
    }

	if(!al_install_audio()){
    fprintf(stderr, "failed to initialize audio!\n");
   
    }

    if(!al_init_acodec_addon()){
    fprintf(stderr, "failed to initialize audio codecs!\n");
    
    }

    if (!al_reserve_samples(3)){ // Reservamos 3 "canais" de áudio
    fprintf(stderr, "failed to reserve samples!\n");
    
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());

    //-------------------------------------------------------------- INICIALIZACAO------------------------------------------------------------

    Nave nave;
    Alien inimigos[4][5];
    Tiro tiro;
    Pontuacao pontuacao;

    initNave(&nave);
    init_horda(inimigos);
    init_pontuacao(&pontuacao);

    //--------------------------------------------------------- CARREGANDO SPRITE E SONS-----------------------------------------------------

    fundo = al_load_bitmap("fundo_espaco.png");
    if(!fundo) {
        fprintf(stderr, "falha ao carregar a imagem de fundo!\n");
        return -1;
    }

    sprite_da_nave = al_load_bitmap("navesprite.png");
    if(!sprite_da_nave) {
        fprintf(stderr, "Falha ao carregar o sprite da nave!\n");
        return -1;
    }

    chao_sprite = al_load_bitmap("grass.png");
    if(!chao_sprite) {
        fprintf(stderr, "Falha ao carregar o sprite do chao!\n");
        return -1;
    }

    alien_spritesheet = al_load_bitmap("catalogo_inimigos.png");
    if(!alien_spritesheet) {
        fprintf(stderr, "Falha ao carregar o spritesheet dos aliens!\n");
        return -1;
    }

    tela_menu = al_load_bitmap("menu.png");
    if(!tela_menu) {
        fprintf(stderr, "Falha ao carregar a tela de menu!\n");
        return -1;
    }

    tela_fim_de_jogo = al_load_bitmap("fimdejogo.png");
    if(!tela_fim_de_jogo) {
        fprintf(stderr, "Falha ao carregar a tela de fim de jogo!\n");
        return -1;
    }

    explosao_sprite = al_load_bitmap("explosao.png");
    if(!explosao_sprite) {
        fprintf(stderr, "Falha ao carregar explosao.png\n");
        return -1;
    }

    sprite_bala = al_load_bitmap("tiro.png");
    if(!sprite_bala) {
        fprintf(stderr, "Falha ao carregar tiro.png\n");
        return -1;
    }

	som_tiro = al_load_sample("som_tiro.wav");
    if(!som_tiro) {
        fprintf(stderr, "Aviso: Falha ao carregar som do tiro\n");
    }

    som_explosao = al_load_sample("som_explosao.wav");
    if(!som_explosao) {
        fprintf(stderr, "Aviso: Falha ao carregar som da explosao\n");
    }

    musica_fundo = al_load_audio_stream("musica_fundo.ogg", 4, 1024);
    if(!musica_fundo) {
        fprintf(stderr, "Aviso: Falha ao carregar musica de fundo\n");
    } else {
        al_attach_audio_stream_to_mixer(musica_fundo, al_get_default_mixer());
        al_set_audio_stream_playmode(musica_fundo, ALLEGRO_PLAYMODE_LOOP);
    }

  //-----------------------------------------------------------------------------------------------------------------------------

    init_tiro(&tiro, sprite_bala, explosao_sprite);
	

    al_start_timer(timer);

    bool rodando = true;
    bool redraw = true;

    while(rodando) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            rodando = false;
        }

        switch(estado_atual) {
            case MENU:
                if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
                    if(ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                        estado_atual = JOGANDO;
                        // Reinicializar o jogo
                        init_horda(inimigos);
                        pontuacao.pontuacao_atual = 0;
                        horda_vel_x = 2.8;
                        tiro.ativo = false;
                        tiro.explodindo = false;
                        redraw = true;
						al_set_audio_stream_playing(musica_fundo, true);
                    }
                }
				        

            case JOGANDO:
                if(ev.type == ALLEGRO_EVENT_TIMER) {
                    update_nave(&nave);
                    update_horda(inimigos, &horda_vel_x);
                    update_tiro(&tiro);

                    if(verifica_colisao_tiro_alien(&tiro, inimigos)) {
                        pontuacao.pontuacao_atual += 10;
						al_play_sample(som_explosao, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    }

                    if(verifica_colisao_horda_solo(inimigos) || verifica_colisao_alien_nave(inimigos, &nave)) {
                     update_recorde(&pontuacao);
                        estado_atual = FIM_DE_JOGO;
                    }

                    if(todos_aliens_mortos(inimigos)) {
                        // Vitória - ir para tela de fim de jogo
                        pontuacao.pontuacao_atual += 100; // Bônus por limpar a tela
                     update_recorde(&pontuacao);
                        estado_atual = FIM_DE_JOGO;
                    }

                    redraw = true;
                }
                else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
                    switch(ev.keyboard.keycode) {
                        case ALLEGRO_KEY_A: nave.esq = 1; break;
                        case ALLEGRO_KEY_D: nave.dir = 1; break;
                        case ALLEGRO_KEY_SPACE:
                            if(!tiro.ativo) {
                                tiro.x = nave.x + (NAVE_W / 2) - (TIRO_W / 2);
                                tiro.y = nave.y;
                                tiro.ativo = true;
								al_play_sample(som_tiro, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                            }
                            break;
                    }
                }
                else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
                    switch(ev.keyboard.keycode) {
                        case ALLEGRO_KEY_A: nave.esq = 0; break;
                        case ALLEGRO_KEY_D: nave.dir = 0; break;
                    }
                }
                break;

            case FIM_DE_JOGO:
                if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
                    if(ev.keyboard.keycode == ALLEGRO_KEY_R) {
                        // Reiniciar completamente o jogo
                        estado_atual = JOGANDO;
                        init_horda(inimigos);
                        pontuacao.pontuacao_atual = 0;
                        horda_vel_x = 2.8;
                        tiro.ativo = false;
                        tiro.explodindo = false;
                        nave.x = SCREEN_W/2 - NAVE_W/2; // Reposicionar nave
                        redraw = true;
                    }
                    else if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                        rodando = false;
                    }
                }
                break;
        }

        if(redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;

            if(estado_atual == MENU) {
             draw_menu(tela_menu, font);
            }
            else if(estado_atual == JOGANDO) {
                draw_scenario(fundo);
                int chao_y = SCREEN_H - al_get_bitmap_height(chao_sprite);
                al_draw_bitmap(chao_sprite, 0, chao_y, 0);
                draw_horda(inimigos, alien_spritesheet);
                draw_tiro(tiro);
                draw_nave(nave);
                draw_pontuacao(pontuacao, font);
            }
            else if(estado_atual == FIM_DE_JOGO) {
             draw_fim_de_jogo(tela_fim_de_jogo, pontuacao, font);
            }

            al_flip_display();
        }
    }

    // Limpeza
    al_destroy_bitmap(alien_spritesheet);
    al_destroy_bitmap(fundo);
    al_destroy_bitmap(sprite_da_nave);
    al_destroy_bitmap(chao_sprite);
    al_destroy_bitmap(explosao_sprite);
    al_destroy_bitmap(sprite_bala);
    al_destroy_bitmap(tela_menu);
    al_destroy_bitmap(tela_fim_de_jogo);
    al_destroy_sample(som_tiro);
    al_destroy_sample(som_explosao);
    al_destroy_audio_stream(musica_fundo);

    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    al_destroy_font(font);

    return 0;
}