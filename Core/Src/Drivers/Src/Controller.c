#include "Controller.h"

void PID_Vel(data * value, uint8_t ts) {
    float error = value->setpoint - value->angle;
    float up = 0, ud = 0;
    static float ui, errorReset, pre_error;
    int64_t pwm;

    int64_t Hlim = 100;
    int64_t Llim = -100;

    float Kp = 32.6683;
    float Ki = 30.7314;
    float Kd = 1.9369;
    float Kb = 3.9833;

    up = Kp * error;
    ui += Ki*error*ts*0.001 + Kb*errorReset*ts*0.001;
    ud = Kd*(error - pre_error)/ts;
    pre_error = error;

    int64_t uout = (int64_t)(up + ui + ud);

    if (uout > 0) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);

        if (uout > Hlim) pwm = Hlim;
        else pwm = uout;

        errorReset = pwm - uout;
    }
    else if (uout < 0) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);

        if (uout < Llim) pwm = Llim;
        else pwm = uout;

        errorReset = pwm - uout;

        pwm *= -1;
    }
    else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);    
        pwm = 0;

        errorReset = pwm - uout;
    }

    TIM3->CCR1 = pwm;
}
