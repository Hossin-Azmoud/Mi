#include <mi.h>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

pthread_mutex_t Mutx;
static float max_amp;

void DistroyPlayerMutex()
{
  pthread_mutex_destroy(&Mutx);
}

void InitPlayerLock()
{
  pthread_mutex_init(&Mutx, NULL);
}

void LockPlayer()
{
  pthread_mutex_lock(&Mutx);
}

void unLockPlayer()
{
  pthread_mutex_unlock(&Mutx);
}
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
  
  MiAudioPlayer *player = (MiAudioPlayer *) pDevice->pUserData;
  MiAudio *audio = &player->audio; 
  ma_uint32 channels = pDevice->playback.channels;

  LockPlayer();
  if (audio == NULL || player->quit) 
        return;

  ma_uint32 leftSamples;
  // (1 frame = 2 samples) in Stereo.
  // so we need to multiply the totalFrames by channels to get the last sample position
  if (audio->position >= audio->totalFrames * channels) {
    rewind_audio(player);
    leftSamples = audio->totalFrames * channels;
  } else {
    leftSamples = audio->totalFrames * channels - audio->position;
  }

  audio->framecount = ((frameCount > leftSamples) ? leftSamples : frameCount);

  if (leftSamples > 0 && player->play)
  {
    float* out = (float*)pOutput;
    ma_uint32 sz = (audio->framecount * channels);
    // memcpy(audio->spectrum, audio->samples + audio->position, sz); 
    max_amp = 0.0f;
    for (size_t k = 0; k < sz; k++) {
      float f = *((audio->samples + audio->position) + k); 
      out[k] =  f * player->volume;
      if (f > max_amp) max_amp = f;
    }

    // TODO: Copy the samples to be visualized, 
    audio->position += sz;
    audio->spec_sz   = sz;
    // audio->framecount = sz/channels;
  } 
  (void)pInput; // Unused.
  unLockPlayer();
}
MiAudioPlayer *init_player(char *file)
{
  MiAudioPlayer *player = (MiAudioPlayer *)malloc(sizeof(MiAudioPlayer));
  MiAudio       *audio  = &player->audio;
  ma_result result;
  memset(player, 0, sizeof(*player));
  strcpy(player->file, file);

  // Init configuration.
  player->volume = 1.0f;
  player->play   = 0;
  player->quit   = 0;

  ma_decoder_config decoderConfig;  
  decoderConfig = ma_decoder_config_init(ma_format_f32, 2, 48000); // Stereo, 48kHz
  result = ma_decoder_init_file(player->file, &decoderConfig, &(audio->decoder));

  if (result != MA_SUCCESS) {
    printf("Failed to initialize decoder.\n");
    return NULL;
  }

  // Speaker, Headphones..
  // initialize the device that will play the audio.
  ma_device_config deviceConfig  = ma_device_config_init(ma_device_type_playback);
  deviceConfig.playback.format   = player->audio.decoder.outputFormat;
  deviceConfig.playback.channels = player->audio.decoder.outputChannels;
  deviceConfig.sampleRate        = player->audio.decoder.outputSampleRate;
  deviceConfig.dataCallback      = data_callback;
  deviceConfig.pUserData         = player;
  player->audio.srate            = player->audio.decoder.outputSampleRate;

  // Loads the audio samples into memory (audio->samples);
  init_audio(audio);
  
  // Sets up a device for playback audio then handes over the callback and decodeer and shared data.
  result = ma_device_init(NULL, &deviceConfig, &player->dev);
  if (result != MA_SUCCESS) {
    printf("Failed to initialize playback device.\n");
    ma_decoder_uninit(&player->audio.decoder);
    return NULL;
  }

  start_playing(player);
  return (player);
}

void deallocate_player(MiAudioPlayer *player)
{
  ma_device_uninit(&player->dev);
  ma_decoder_uninit(&player->audio.decoder);
  // endwin(); We dont end the window here because we still need it..
  free(player->audio.samples);
  free(player);
}

void start_playing(MiAudioPlayer *player)
{
  ma_result result = ma_device_start(&player->dev);
    if (result != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&player->dev);
        ma_decoder_uninit(&player->audio.decoder);
      exit(1);
  }
}

void player_play(MiAudioPlayer *player)
{
  player->play = 1;
}

void player_pause(MiAudioPlayer *player)
{
  player->play = 0;
}

void toggle_play(MiAudioPlayer *player)
{
  if (player->play) {
      player_pause(player);
    return;
  }
  player_play(player);
}

void seek_position(MiAudioPlayer *player, int offset_in_secs)
{
  ma_uint64 sr = player->audio.srate;
  MiAudio  *audio = &player->audio;
  int moveoffsetframes = (offset_in_secs * sr);
  if (moveoffsetframes > 0) {
  
    // Move to the left.
    if (player->audio.position + moveoffsetframes >=  player->audio.totalFrames * audio->totalFrames * audio->decoder.outputChannels) {
      moveoffsetframes = (player->audio.totalFrames - player->audio.position);
    }

    if (player->audio.position < player->audio.totalFrames * audio->totalFrames * audio->decoder.outputChannels) {
      player->audio.position += moveoffsetframes; // Basically seek to 5 seconds after;
    }
  } else {
    // Move backwards to the right.
    if ((int)player->audio.position + (int64_t)moveoffsetframes < 0) {
      moveoffsetframes = player->audio.position; 
    }

    if (player->audio.position > 0) {
      player->audio.position += moveoffsetframes; // Basically rewind 5 seconds
    }
  }
}

void rewind_audio(MiAudioPlayer *player)
{
  player->audio.position = 0; // Replay.
}

                         
void volume(MiAudioPlayer *player, float new_volume)
{
  player->volume = new_volume;

  if (new_volume > 1.0f)
    player->volume = 1.0f;
  if (new_volume < 0.0f)
    player->volume = 0.0f;
}



void *player_visualize_audio(void *E)
{ 
  // We load the audio in this thread for some reasonl
  LockPlayer();
  MiEditor *Ed = (MiEditor *) E;
  MiAudioPlayer *player = (MiAudioPlayer *) Ed->mplayer;  
  MiAudio       *audio  = &player->audio;
  int h, w;

  player->play = 1;
  unLockPlayer();
  curs_set(0);
  int viw;
  int vih;
  
  while (!(player->quit))
  {
    h = Ed->renderer->win_h;
    w = Ed->renderer->win_w;
    viw = ((w/10)*7); // 70% of the width
    vih = ((h/10)*3); // 40% of the height
    // Chords to use in order to visualize the spectrum and the configurations (volume, timestamp...)
    int ystart = (h/2+vih/2), xstart = (w/2-viw/2), yend = (h/2+vih/2), xend;
    erase(); 

    
    
    
    LockPlayer();
    // render the possible freqs
    int N = audio->framecount;

    if (N > viw)
      N = viw;

    // Apply hann window and fft
    // player->audio.fft_ = (float complex *) malloc(sizeof(float complex) * N);
    // apply_hamming_window(player->audio.spectrum, N);
    // fft(player->audio.spectrum, 
       // 1, 
        // audio->fft_, 
        // N);
    // get_spectrum(audio->fft_, player->audio.spectrum, player->audio.spec_sz, N, player->audio.srate);

    float t1 = 0;
    for (int k = 0, x = xstart; k < N; k++, x++) {
      t1 = fabsf(*((audio->samples + audio->position) + k))/max_amp * vih;
      for (int y = ystart; (y > yend - t1); --y) {
        mvaddch(y, x, ' ');
        mvchgat(y, x, 1, A_NORMAL, 1, NULL);
      }
      xend = x;
    }
    // Bar of audio progress.
    int current = 
      (get_frames_as_seconds(&player->audio.decoder, player->audio.position) / (player->audio.duration)) * viw;
    
    for (int x = 0; (x < current); ++x)
      mvaddch(ystart + 2, (xstart + x), ' ');
    mvchgat(ystart + 2, xstart, current, A_NORMAL, HIGHLIGHT_WHITE, NULL);
    
    current = player->volume * vih;
    for (int y = ystart; y > yend - current; y--) 
    {
        mvaddch(y, xend + 2, ' ');
        mvchgat(y, xend + 2, 1, A_NORMAL, HIGHLIGHT_WHITE, NULL);
    }
    
    mvprintw(ystart + 4, xstart, "Now playing %s [%f] | [%f]", player->file, player->audio.duration, get_frames_as_seconds(&player->audio.decoder, player->audio.position));
    mvprintw(ystart + 6, xstart, "%i channels [%iHz]",  player->audio.decoder.outputChannels, player->audio.decoder.outputSampleRate);
    mvprintw(ystart + 7, xstart, "VOL         %f", player->volume); 
    mvprintw(ystart + 8, xstart, "Controls    [w = vol up, s = vol down, d = seek forward, a = seek backwards, q = quit, r = rewind, space = pause/play]");

    // Render whatever the editor needs to render.
    // We dont need a refresh call because the editor will take care of it.
    editor_render(Ed);
    if (player->audio.fft_)
      free(player->audio.fft_);

    if (player->audio.spec_sz || player->audio.spectrum) {
      // memset(player->audio.spectrum, 0, player->audio.spec_sz);
      player->audio.spec_sz = 0;
    }
    unLockPlayer();
  }
  return NULL;
}

// TODO: make the volume editable using left and right arrow keys.
void editor_init_player_routine(MiEditor *E, char *mp3_file)
{
  E->mplayer = init_player(mp3_file);
  E->mode = MPLAYER; // Set the mode to music player.
  pthread_t playerThread;
  pthread_t updateThread;
  pthread_create(&updateThread, NULL,   editor_player_update__internal, E);
  pthread_create(&playerThread, NULL, player_visualize_audio, E);
  
  pthread_join(updateThread, NULL);
  pthread_join(playerThread, NULL);
  
  // Clean
  pthread_cancel(playerThread);
  pthread_cancel(updateThread);
  deallocate_player(E->mplayer);
  DistroyPlayerMutex();
}

void *editor_player_update__internal(void *E) 
{
  MiEditor *Ed = (MiEditor *) E;
  MiAudioPlayer *player = (MiAudioPlayer *) Ed->mplayer;
  int c = 0;
  while (!player->quit) {
    c = getchar();
    LockPlayer();
    switch(c) {
    case 'q': {
      player->quit = 1;
    } break;
    case 'w': {
      volume(player, player->volume + .1f);
    } break;
    case 's': {
      volume(player, player->volume - .1f);
    } break;
    case ' ': {
      toggle_play(player);
    } break;
    case 'a': {
      seek_position(player, -5);
    } break;
    case 'd': {
      seek_position(player, 5);
    } break;
    case 'r': { // rezero.
      rewind_audio(player);
    } break;
    }
    unLockPlayer();
  }
  return NULL;
}

void editor_player_update(MiEditor *E, int c)
{
  MiAudioPlayer *player = (MiAudioPlayer *) E->mplayer;
  LockPlayer();
    
  switch(c) {
    case 'q': {
      player->quit = 1;
    } break;
    case 'w': {
      volume(player, player->volume + .1f);
    } break;
    case 's': {
      volume(player, player->volume - .1f);
    } break;
    case ' ': {
      toggle_play(player);
    } break;
    case 'a': {
      seek_position(player, -5);
    } break;
    case 'd': {
      seek_position(player, 5);
    } break;
    case 'r': { // rezero.
      rewind_audio(player);
    } break;
  }

  unLockPlayer();
}
