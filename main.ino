#include<String.h>

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_core = 0;
#else 
static const BaseType_t app_core = 1;
#endif

static hw_timer_t *timer;
static QueueHandle_t que1;
static QueueHandle_t que2;
static SemaphoreHandle_t mutavg;

int analog_data;
static String str;
static uint32_t Avg = 0;
void IRAM_ATTR ADCread()
{
  analog_data = analogRead(4);
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  // Mutexes are illegal in ISRs, but Queues are already inherently safe!
  // Try sending to que1 using the safe ISR function
  xQueueSendFromISR(que1, &analog_data, &xHigherPriorityTaskWoken);
  // Yield if a higher priority task was woken up
}
void taskA (void *parameter)
{
  
  vTaskDelay(1000/portTICK_PERIOD_MS);
  uint32_t tempAvg = 0,temp=0;
  while(1)
  {
    if(uxQueueSpacesAvailable(que1) != pdTRUE)
    {
     while((xQueueReceive(que1, &temp, 50/portTICK_PERIOD_MS)) == pdTRUE)
     {
      tempAvg = tempAvg +temp;
     }
      xSemaphoreTake(mutavg, portMAX_DELAY);
      Avg = tempAvg / 10;
      tempAvg = 0;
      xSemaphoreGive(mutavg);
    }
  }
}
void taskB(void *parameter)
{
  while(1)
  {
  str =  Serial.readStringUntil('\n');
  str.trim();
  if(str == "avg")
  { 
  xSemaphoreTake(mutavg, portMAX_DELAY);
  Serial.println(Avg);
  xSemaphoreGive(mutavg);
  }
  else
  Serial.println(str);
  }
}
void setup()
{
  Serial.begin(115200);  
  vTaskDelay(500/portTICK_PERIOD_MS);

//###########################mutex#################################

  mutavg  = xSemaphoreCreateMutex();
//###########################Queue#################################
  que1 = xQueueCreate(10, 32);
  que2 = xQueueCreate(10, 32);
  xTaskCreatePinnedToCore(taskA, "adc avg", 1024, NULL, 2, NULL, app_core);
  xTaskCreatePinnedToCore(taskB, "adc avg display", 1024, NULL, 1, NULL, app_core);
//########################### timer ###############################
  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, ADCread);
  timerAlarm(timer, 100000, true, 20);
  timerStart(timer); 
}
void loop()
{}
