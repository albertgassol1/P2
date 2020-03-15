#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sndfile.h>

#include "vad.h"
#include "vad_docopt.h"

#define DEBUG_VAD 0x1

int main(int argc, char *argv[]) {
  int verbose = 0; /* To show internal state of vad: verbose = DEBUG_VAD; */
  SNDFILE *sndfile_in, *sndfile_out = 0;
  SF_INFO sf_info;
  FILE *vadfile;
  int n_read = 0, i, j;
  int n_maybe = 0;

  VAD_DATA *vad_data;
  VAD_STATE state, last_state, aux_state;

  float *buffer, *buffer_zeros;
  int frame_size;         /* in samples */
  float frame_duration;   /* in seconds */
  unsigned int t, last_t, start_t; /* in frames */


  char	*input_wav, *output_vad, *output_wav, *alpha1, *alpha2, *frame_silence, *frame_voice, *zeros;

  DocoptArgs args = docopt(argc, argv, /* help */ 1, /* version */ "2.0");

  verbose    = args.verbose ? DEBUG_VAD : 0;
  input_wav  = args.input_wav;
  output_vad = args.output_vad;
  output_wav = args.output_wav;

  // Check if arguments aren't NULLS
  if(args.alpha1){

    alpha1 = args.alpha1;
  }else{

    alpha1 = "4";
  }

  if(args.alpha2){

    alpha2 = args.alpha2;
  }else{

    alpha2 = "1";
  }

  if(args.frame_silence){

    frame_silence = args.frame_silence;
  }else{

    frame_silence = "6";
  }
  
  if(args.frame_voice){

    frame_voice = args.frame_voice;
  }else{

    frame_voice = "8";
  }

  if(args.zeros){

    zeros = args.zeros;
  }else{

    zeros = "1900";
  }

  if (input_wav == 0 || output_vad == 0) {
    fprintf(stderr, "%s\n", args.usage_pattern);
    return -1;
  }

  /* Open input sound file */
  if ((sndfile_in = sf_open(input_wav, SFM_READ, &sf_info)) == 0) {
    fprintf(stderr, "Error opening input file %s (%s)\n", input_wav, strerror(errno));
    return -1;
  }

  if (sf_info.channels != 1) {
    fprintf(stderr, "Error: the input file has to be mono: %s\n", input_wav);
    return -2;
  }

  /* Open vad file */
  if ((vadfile = fopen(output_vad, "wt")) == 0) {
    fprintf(stderr, "Error opening output vad file %s (%s)\n", output_vad, strerror(errno));
    return -1;
  }

  /* Open output sound file, with same format, channels, etc. than input */
  if (output_wav) {
    if ((sndfile_out = sf_open(output_wav, SFM_WRITE, &sf_info)) == 0) {
      fprintf(stderr, "Error opening output wav file %s (%s)\n", output_wav, strerror(errno));
      return -1;
    }
  }
  vad_data = vad_open(sf_info.samplerate, alpha1, alpha2, frame_silence, frame_voice, zeros);
  /* Allocate memory for buffers */
  frame_size   = vad_frame_size(vad_data);
  buffer       = (float *) malloc(frame_size * sizeof(float));
  buffer_zeros = (float *) malloc(frame_size * sizeof(float));
  for (i=0; i< frame_size; ++i) buffer_zeros[i] = 0.0F;

  frame_duration = (float) frame_size/ (float) sf_info.samplerate;
  last_state = ST_UNDEF;
  aux_state = ST_UNDEF;

  for (t = last_t = 0; ; t++) { /* For each frame ... */
    /* End loop when file has finished (or there is an error) */
    if  ((n_read = sf_read_float(sndfile_in, buffer, frame_size)) != frame_size) break;

    if (sndfile_out != 0) {
      /* TODO: copy all the samples into sndfile_out */
      sf_write_float(sndfile_out, buffer, frame_size);
    }

    state = vad(vad_data, buffer);
    if(state == ST_MAY_SILENCE || state == ST_MAY_VOICE){
      n_maybe++;
    }

    if (verbose & DEBUG_VAD) vad_show_state(vad_data, stdout);

    /* TODO: print only SILENCE and VOICE labels */

    if (state != last_state) {
      if (t != last_t){

        if((last_state == ST_VOICE || last_state == ST_SILENCE) && (state == ST_MAY_SILENCE || state == ST_MAY_VOICE)){

          start_t = t;
        }else{

          if(last_state == ST_MAY_VOICE && state == ST_VOICE){

            fprintf(vadfile, "%.5f\t%.5f\t%s\n", last_t * frame_duration, start_t * frame_duration, state2str(ST_SILENCE));
            last_t = start_t;
          }

          if (last_state == ST_MAY_SILENCE && state == ST_SILENCE)
          {
          
            fprintf(vadfile, "%.5f\t%.5f\t%s\n", last_t * frame_duration, start_t * frame_duration, state2str(ST_VOICE));
            last_t = start_t;
          }

        }
          
      }
      aux_state = last_state;
      last_state = state;
      
        
    }

    if (sndfile_out != 0) {
      /* TODO: go back and write zeros in silence segments */

      if(state == ST_SILENCE && aux_state == ST_SILENCE){
        
        sf_seek(sndfile_out,-frame_size, SEEK_CUR);
        sf_write_float(sndfile_out, buffer_zeros, frame_size);
        
      }else if (state == ST_UNDEF && aux_state == ST_UNDEF)
      {
        sf_seek(sndfile_out,-frame_size, SEEK_CUR);
        sf_write_float(sndfile_out, buffer_zeros, frame_size);

      }else if (state == ST_SILENCE && aux_state == ST_UNDEF)
      {
        sf_seek(sndfile_out,-frame_size, SEEK_CUR);
        sf_write_float(sndfile_out, buffer_zeros, frame_size);

      }else if (state == ST_SILENCE && aux_state != ST_SILENCE)
      {
        sf_seek(sndfile_out, -frame_size*(n_maybe + 1), SEEK_CUR);
        
        for(j = 0; j < n_maybe + 1 ; j++){

          sf_write_float(sndfile_out, buffer_zeros, frame_size);
        }

        n_maybe = 0;
      }else if (state == ST_VOICE && aux_state != ST_SILENCE){
        
        n_maybe = 0;
      }
      
    }
    

  }

  state = vad_close(vad_data);
  /* TODO: what do you want to print, for last frames? */
  if (t != last_t)
    if(state == ST_VOICE){
      
      fprintf(vadfile, "%.5f\t%.5f\t%s\n", last_t * frame_duration, t * frame_duration + n_read / (float) sf_info.samplerate, state2str(state));
      if (sndfile_out != 0) {

        sf_write_float(sndfile_out, buffer, n_read);
      }
    }
    else{

      fprintf(vadfile, "%.5f\t%.5f\t%s\n", last_t * frame_duration, t * frame_duration + n_read / (float) sf_info.samplerate, state2str(ST_SILENCE));
      if (sndfile_out != 0) {

        sf_write_float(sndfile_out, buffer_zeros, n_read);
      }
    }
    
  

    
    

  /* clean up: free memory, close open files */
  free(buffer);
  free(buffer_zeros);
  sf_close(sndfile_in);
  fclose(vadfile);
  if (sndfile_out) sf_close(sndfile_out);
  return 0;
}
