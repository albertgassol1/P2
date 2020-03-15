#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "vad.h"


float alpha1 = 3;
float alpha2 = 1;
int frames_silence = 3; 
int frames_voice = 7;
int n_init;
const float FRAME_TIME = 10.0F; /* in ms. */

/* 
 * As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
 * only this labels are needed. You need to add all labels, in case
 * you want to print the internal state in string format
 */

const char *state_str[] = {
  "UNDEF", "S", "V", "INIT"
};

const char *state2str(VAD_STATE st) {
  return state_str[st];
}

/* Define a datatype with interesting features */
typedef struct {
  float zcr;
  float p;
  float am;
} Features;



Features compute_features(const float *x, int N, float fm) {

  Features feat;
  feat.p = compute_power(x, N);
  feat.am = compute_am(x, N);
  feat.zcr = compute_zcr(x, N, fm);
  return feat;
}



VAD_DATA *vad_open(float rate, char *_alpha1, char *_alpha2, char *_frame_silence, char *_frame_voice, char *_zeros) {

  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;
  vad_data->count_silence = 0;
  vad_data->count_voice = 0;
  n_init = 0;
  alpha1 = (float) strtod(_alpha1,NULL);
  alpha2 = (float) strtod(_alpha2,NULL);
  frames_silence =  atoi(_frame_silence);
  frames_voice =  atoi(_frame_voice);
  vad_data->zeros = (float) strtod(_zeros,NULL);
  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {

  VAD_STATE state = vad_data->state;

  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

/* 
 * TODO: Implement the Voice Activity Detection 
 * using a Finite State Automata
 */

VAD_STATE vad(VAD_DATA *vad_data, float *x) {



  Features f = compute_features(x, vad_data->frame_length, vad_data->sampling_rate);
  vad_data->last_feature = f.p;

  switch (vad_data->state) {

  case ST_INIT:
    
    if (n_init < 10){
        
        k0 += pow(10, f.p/10);
        n_init ++;
        
        if(n_init == 10){

          k0 = 10*log10(k0/n_init);
          vad_data->state = ST_SILENCE;
          vad_data->k1 = k0 + alpha1;
          vad_data->k2 = vad_data->k1 + alpha2;
        }
    }

    break;

  case ST_SILENCE:

    if (f.p > vad_data->k1)
      vad_data->state = ST_MAY_VOICE;

    break;

  case ST_VOICE:

    if (f.p < vad_data->k2)
      vad_data->state = ST_MAY_SILENCE;

    break;

  case ST_MAY_SILENCE:

      if(f.p > vad_data->k1 || f.zcr > vad_data->zeros){
        
        vad_data->state = ST_VOICE;
        vad_data->count_silence = 0;
      
      } else {
          
        vad_data->count_silence++;
        if(vad_data->count_silence == frames_silence){

          vad_data->state = ST_SILENCE;
          vad_data->count_silence = 0;
        }
      }
    break;

  case ST_MAY_VOICE:

     if(f.p < vad_data->k2 && f.zcr < vad_data->zeros){
        
        vad_data->state = ST_SILENCE;
        vad_data->count_voice = 0;

      } else {
          
          vad_data->count_voice++;
          
          if(vad_data->count_voice == frames_voice){
            vad_data->count_voice = 0;
            vad_data->state = ST_VOICE;
          }
      }

    break;

  case ST_UNDEF:

    break;

  }

  if (vad_data->state == ST_SILENCE ||
      vad_data->state == ST_VOICE ||
      vad_data->state == ST_MAY_SILENCE ||
      vad_data->state == ST_MAY_VOICE)
    return vad_data->state;
  else
    return ST_UNDEF;
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}
