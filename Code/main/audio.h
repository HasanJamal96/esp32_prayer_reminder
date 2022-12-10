void play(Output *output, const char *fname) {
  int16_t *samples = (int16_t *)malloc(sizeof(int16_t) * 1024);
  FILE *fp = fopen(fname, "rb");
  if(fp == NULL) {
    #if (DEBUG_MAIN == true)
      Serial.println("[Prayer] Audio file doesnot exist");
    #endif
    return;
  }
  WAVFileReader *reader = new WAVFileReader(fp);
  output->start(reader->sample_rate());
  while (playingAudio) {
    int samples_read = reader->read(samples, 1024);
    if (samples_read == 0)
      break;
    output->write(samples, samples_read);
  }
  output->stop();
  fclose(fp);
  playingAudio = false;
  delete reader;
  free(samples);
}
