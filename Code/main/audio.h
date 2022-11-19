void play(Output *output, const char *fname) {
  int16_t *samples = (int16_t *)malloc(sizeof(int16_t) * 1024);
  FILE *fp = fopen(fname, "rb");
  ESP_LOGI(TAG, "Start playing");
  WAVFileReader *reader = new WAVFileReader(fp);
  ESP_LOGI(TAG, "Opened wav file");
  output->start(reader->sample_rate());
  while (true) {
    int samples_read = reader->read(samples, 1024);
    if (samples_read == 0)
      break;
    ESP_LOGI(TAG, "Read %d samples", samples_read);
    output->write(samples, samples_read);
  }
  output->stop();
  fclose(fp);
  delete reader;
  free(samples);
  ESP_LOGI(TAG, "Finished playing");
}
