//Arduino Pin setup
const int EMG_1_PIN = A0;
const int EMG_2_PIN = A1;
const int VIBRATION_1_PIN = 5;
const int VIBRATION_2_PIN = 6;

//Variable initiations
int current_threshold = 200;
int attempts_at_curr_threshold = 0;
int correct_attempts_at_curr_threshold = 0;
const int THRESHOLD_STEP = 25;
const int MIN_ATTEMPTS = 15;
const float NECESSARY_SUCCESS_RATE = 80.0;
bool use_vibration = true;
bool need_new_task = true;
const unsigned long VIBRATION_TIME = 500;         // time the vibration runs for
const unsigned long RESPONSE_TIME_ALLOWED = 2000; // time allowed to perform the task
const unsigned long REST_TIME = 1000;             // time between attempts

//Initiating task options
enum Task {TASK_EMG1, TASK_EMG2, TASK_BOTH};
Task current_task;

//Helper func for naming tasks
const char* taskName(Task t) {
  switch (t) {
    case TASK_EMG1: return "EMG1";
    case TASK_EMG2: return "EMG2";
    default:        return "Both EMGs";
  }
}

//Helper func for making buzzer vibrate
void vibrate(Task t) {
  if (!use_vibration) return;
  if (t == TASK_EMG1 || t == TASK_BOTH) digitalWrite(VIBRATION_1_PIN, HIGH);
  if (t == TASK_EMG2 || t == TASK_BOTH) digitalWrite(VIBRATION_2_PIN, HIGH);
  delay(VIBRATION_TIME);
  digitalWrite(VIBRATION_1_PIN, LOW);
  digitalWrite(VIBRATION_2_PIN, LOW);
}

//Helper func for reading in EMGs
void readEMGs(bool &emg1Active, bool &emg2Active) {
  emg1Active = false;
  emg2Active = false;
  unsigned long start = millis();
  while (millis() - start < RESPONSE_TIME_ALLOWED) {
    if (analogRead(EMG_1_PIN) > current_threshold) emg1Active = true;
    if (analogRead(EMG_2_PIN) > current_threshold) emg2Active = true;
  }
}

//Helper func for determining if emg read in is correct for the task
bool isCorrect(Task t, bool emg_1, bool emg_2) {
  switch (t) {
    case TASK_EMG1: return  emg_1 && !emg_2;
    case TASK_EMG2: return !emg_1 &&  emg_2;
    case TASK_BOTH: return  emg_1 &&  emg_2;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  pinMode(VIBRATION_1_PIN, OUTPUT);
  pinMode(VIBRATION_2_PIN, OUTPUT);
  randomSeed(analogRead(A5));
}

void loop() {
  //Generate next task
  if (need_new_task) {
    current_task = (Task)random(0, 3);   // 0, 1, or 2
    need_new_task = false;
  }
  Serial.println(taskName(current_task));
  vibrate(current_task);

  //Read in EMGs and check if threshold was passed
  bool emg_1;
  bool emg_2;
  readEMGs(emg_1, emg_2);

  //Evaluate performance
  attempts_at_curr_threshold++;
  if (isCorrect(current_task, emg_1, emg_2)) {
    Serial.println(String("ACTION: ") + taskName(current_task));
    correct_attempts_at_curr_threshold++;
    need_new_task = true;
  } else {
    Serial.println("NO_MOVE");
  }

  //Check if threshold needs to be moved
  float percent_success = 100.0 * correct_attempts_at_curr_threshold / attempts_at_curr_threshold;
  if (attempts_at_curr_threshold > MIN_ATTEMPTS && percent_success > NECESSARY_SUCCESS_RATE) {
    current_threshold += THRESHOLD_STEP;
    attempts_at_curr_threshold = 0;
    correct_attempts_at_curr_threshold = 0;
    Serial.println(String("THRESHOLD_UP: ") + current_threshold);
  }
  delay(REST_TIME);
}
