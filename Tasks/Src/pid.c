/****************************************************************************
 *  Copyright (C) 2019 RoboMaster.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/

#include "includes.h"

void abs_limit(__packed float *a, float ABS_MAX)
{
  if (*a > ABS_MAX)
    *a = ABS_MAX;
  if (*a < -ABS_MAX)
    *a = -ABS_MAX;
}

static void pid_param_init(
    struct pid *pid,
    float maxout,
    float inte_limit,
    float kp,
    float ki,
    float kd)
{

  pid->param.inte_limit = inte_limit;
  pid->param.max_out = maxout;

  pid->param.p = kp;
  pid->param.i = ki;
  pid->param.d = kd;
}

/**
  * @brief     modify pid parameter when code running
  * @param[in] pid: control pid struct
  * @param[in] p/i/d: pid parameter
  * @retval    none
  */
static void pid_reset(struct pid *pid, float kp, float ki, float kd)
{
  pid->param.p = kp;
  pid->param.i = ki;
  pid->param.d = kd;

  pid->pout = 0;
  pid->iout = 0;
  pid->dout = 0;
  pid->output = 0;
}

/**
  * @brief     calculate delta PID and position PID
  * @param[in] pid: control pid struct
  * @param[in] get: measure feedback value
  * @param[in] target: target value
  * @retval    pid calculate output 
  */
float pid_calculate(struct pid *pid, float feedback, float target)
{
  pid->feedback = feedback;
  pid->target = target;
  pid->err = target - feedback;
  if ((pid->param.input_max_err != 0) && (__fabs(pid->err) > pid->param.input_max_err))
    return 0;

  pid->pout = pid->param.p * pid->err;
  pid->iout += pid->param.i * pid->err;
  pid->dout = pid->param.d * (pid->err - pid->last_err);

  abs_limit(&(pid->iout), pid->param.inte_limit);
  pid->output = pid->pout + pid->iout + pid->dout;
  abs_limit(&(pid->output), pid->param.max_out);

  return pid->output;
}
/**
  * @brief     initialize pid parameter
  * @retval    none
  */
void pid_struct_init(
    struct pid *pid,
    float maxout,
    float inte_limit,

    float kp,
    float ki,
    float kd)
{
  pid->f_param_init = pid_param_init;
  pid->f_pid_reset = pid_reset;

  pid->f_param_init(pid, maxout, inte_limit, kp, ki, kd);
  pid->f_pid_reset(pid, kp, ki, kd);
}

int PID_PROCESS_Double(struct pid pid_position, struct pid pid_speed, 
									float target, float position_feedback, float speed_feedback)
{
	//position		
	pid_calculate(&pid_position, position_feedback, target);
	//speed
	pid_calculate(&pid_speed, speed_feedback, pid_position.output);
	return pid_speed.output;
}

