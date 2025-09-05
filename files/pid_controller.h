#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

class PIDController {
public:
    PIDController(float kp, float ki, float kd);
    float compute(float setpoint, float measured_value, float dt);
    void reset();
    void setLimits(float min_output, float max_output);
    void setIntegralLimits(float min_integral, float max_integral);

private:
    float kp_;
    float ki_;
    float kd_;
    float integral_=0.0f; // initialised the values to avoid garbage values
    float previous_error_=0.0f;
    float min_output_=-1e6f;
    float max_output_=1e6f;
    float min_integral_=-1e6f;
    float max_integral_=1e6f;
};

#endif // PID_CONTROLLER_H
