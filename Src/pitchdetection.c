#define WINDOW_SIZE 0.1                                                                                                                                                                                                                      
#define SAMPLE_RATE 5525 //TODO check this                                                                                                                                                                                                   
#define BUF_SIZE 554 //WINDOW_SIZE * SAMPLE_RATE                                                                                                                                                                                             
#define MAX_TAU 400
#define K 0.9      

//frequency calculation and audio buffers
double nsdf_buf[MAX_TAU];
uint16_t audio_buf[BUF_SIZE];

//frequency calculation functions
double predict_freq();
double nsdf(uint16_t tau);
double acr(uint16_t tau);
double m(uint16_t tau);

//Put main() here for testing

//O(MAX_TAU * WINDOW_SIZE * SAMPLE_RATE)
double predict_freq() {

    //iterate over 1 to max tau and calculate NSDF
    //TODO optimize this
    uint16_t tau;
    double curr_max = 0;
    double curr_nsdf;

    for (tau = 1; tau <= MAX_TAU; tau++) {
        curr_nsdf = nsdf(tau);
        nsdf_buf[tau-1] = curr_nsdf;
        if (curr_nsdf > curr_max)
            curr_max = curr_nsdf;
    }
    double threshold = curr_max * K;

    //find first local maxima above threshold
    double max_tau = 0;
    for (tau = 1; tau < MAX_TAU; tau++) {
        double curr = nsdf_buf[tau];
        double prev = nsdf_buf[tau-1];
        double next = nsdf_buf[tau+1];
        if (curr > threshold && prev <= curr && next <= curr) {
            max_tau = curr;
        }
    }

    if (max_tau <= 0) {
        printf("ERROR: No max tau found with threshold %f\n", threshold);
        return 0;
    }

    //translate sample period to Hz and return
    return SAMPLE_RATE / max_tau;
}

//O(O(acr) + O(m))
double nsdf(uint16_t tau) {
    return 2 * acr(tau) / m(tau);
}

//O(window size * sample rate)
double acr(uint16_t tau) {
    int i;
    double sum = 0;
    //calculate each acr value                                                                                                                                                                                                                   for (i = 0; i < BUF_SIZE - 1 - tau; i++) {                                                                                                                                                                                                       sum += audio_buf[i] * audio_buf[i + tau];                                                                                                                                                                                                }

    return sum;
}

                                                                                                                                                                                                                                             //O(window size * sample rate)
double m(uint16_t tau) {
    int i;                                                                                                                                                                                                                                       double sum = 0;
    //calculate each m value                                                                                                                                                                                                                     for (i = 0; i < BUF_SIZE - 1 - tau; i++) {
        sum += audio_buf[i] * audio_buf[i] + audio_buf[i + tau] * audio_buf[i + tau];
    }

    return sum;
}               