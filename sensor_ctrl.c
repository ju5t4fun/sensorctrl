/*
 * =====================================================================================
 *
 *       Filename:  sensor_ctrl.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2016年06月15日 09时33分17秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liuchangjian (), 
 *        Company:  vivo
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

// qcom header
#include <media/msm_cam_sensor.h>

#define DEBUG_HIGH

#ifdef DEBUG_HIGH
#define LOGH(fmt,args...)	printf(fmt,##args)
#define LOGL(fmt,args...)	
#define LOG(fmt,args...)	
#define LOGE(fmt,args...)	printf(fmt,##args)
#elif defined(DEBUG_LOW)
#define LOGH(fmt,args...)	printf(fmt,##args)
#define LOGL(fmt,args...)	printf(fmt,##args)
#define LOG(fmt,args...)	
#define LOGE(fmt,args...)	printf(fmt,##args)
#else
#define LOGH(fmt,args...)	printf(fmt,##args)
#define LOGL(fmt,args...)	printf(fmt,##args)
#define LOG(fmt,args...)	printf(fmt,##args)
#define LOGE(fmt,args...)	printf(fmt,##args)
#endif

#define CAM_PATH	"/dev/v4l-subdev"

#define CAM_SUBDEV_LEN	20

enum{
	NON_FLAG,
	R_FLAG,
	W_FLAG,
}WR_FLAG;

static void useage()
{
	printf("vivo_cam_sensor cmd:\n");
	printf("		vivo_cam_sensor  cam_subdev_num  [reg] [value]");
}

// get cam sensor name 
int cam_sensor_name(int fd)
{
	int rc;
	struct sensorb_cfg_data cfg;
	LOG("%s\n",__func__);	
	cfg.cfgtype = CFG_GET_SENSOR_INFO;
	rc = ioctl(fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
	if (rc < 0) {
	    LOGE("ERROR: failed rc %d", rc);
		return -1;
	}
	LOGH("\nsensor name is %s\n",cfg.cfg.sensor_info.sensor_name);
	LOGH("session id %d\n",cfg.cfg.sensor_info.session_id);
	LOGH("subdev_id  %d\n",cfg.cfg.sensor_info.subdev_id[SUB_MODULE_SENSOR]);
	LOGH("subdev_intf %d\n",cfg.cfg.sensor_info.subdev_intf[SUB_MODULE_SENSOR]);
	return 0;
}

static int cam_read(int fd,int reg)
{
	int rc,val;
	struct sensorb_cfg_data cfg;
	struct msm_camera_i2c_read_config read_config;
	LOG("%s\n",__func__);	

	memset(&read_config,0,sizeof(read_config));
	read_config.slave_addr=0;
	read_config.reg_addr=reg;
	read_config.data_type=MSM_CAMERA_I2C_BYTE_DATA;
    
	cfg.cfgtype = CFG_SLAVE_READ_I2C;
    cfg.cfg.setting = &read_config;

	rc = ioctl(fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
	if (rc < 0) {
	    LOGE("ERROR: msm sensor read failed rc %d", rc);
		return -1;
	}
	val=read_config.data; 
	LOGH("\nsensor read reg 0x%x,val 0x%x!\n",reg,val);
	return 0;
}

static int cam_write(int fd,int reg,int val)
{
	int rc;
	struct sensorb_cfg_data cfg;
	struct msm_camera_i2c_reg_setting setting;
	struct msm_camera_i2c_reg_array reg_array;
	
	LOG("%s",__func__);	

	memset(&reg_array,0,sizeof(reg_array));
	reg_array.reg_addr=reg;
	reg_array.reg_data=val;
	reg_array.delay=0;

	memset(&setting,0,sizeof(setting));
	setting.reg_setting=&reg_array;
	setting.size=1;
	setting.addr_type=MSM_CAMERA_I2C_WORD_ADDR;
	setting.data_type=MSM_CAMERA_I2C_WORD_DATA;
	setting.delay=0;

    cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
    cfg.cfg.setting = &setting;

	rc = ioctl(fd, VIDIOC_MSM_SENSOR_CFG, &cfg);
	if (rc < 0) {
	    LOGE("ERROR: msm sensor write failed rc %d", rc);
		return -1;
	}
	LOGH("\nsensor write reg 0x%x,val 0x%x!\n",reg,val);
	return 0;
}

int main(int argc,char *argv[])
{
	int ret;
	int wr_flag=NON_FLAG;
	int cam_fd;
	char cam_subdev[CAM_SUBDEV_LEN];
	int cam_sub_num;
	int reg=0,reg_val=0;

	LOG("%s running: \n",__func__);
	
	if(1==argc)
	{	
		useage();
		return 0;
	}
	else if (2==argc)
	{
		cam_sub_num=atoi(argv[1]);
	}
	else if(3==argc)
	{
		cam_sub_num=atoi(argv[1]);
		reg=atoi(argv[2]);
		wr_flag=R_FLAG;
	}	
	else if(4==argc)
	{	
		cam_sub_num=atoi(argv[1]);
		reg=atoi(argv[2]);
		reg_val=atoi(argv[3]);
		wr_flag=W_FLAG;
	}	

	snprintf(cam_subdev,CAM_SUBDEV_LEN,"%s%d",CAM_PATH,cam_sub_num);
	LOGH("cam subdev:%s\n",cam_subdev);
	
	cam_fd=open(cam_subdev,O_RDWR | O_NONBLOCK);
	if(cam_fd < 0)
		LOGE("ERROR: Can't open %s.  Res:%s\n",cam_subdev,strerror(errno));

	LOGL("open file:%s\n",cam_subdev);

	if(NON_FLAG==wr_flag)
	{	
		ret=cam_sensor_name(cam_fd);
		if(ret<0)
			goto err;
		else
			return 0;
	}		
	else if(R_FLAG==wr_flag)
		cam_read(cam_fd,reg);
	else if(W_FLAG==wr_flag)
		cam_write(cam_fd,reg,reg_val);

err:
	close(cam_fd);
	LOGL("close file:%s\n",cam_subdev);
	return 0;
}
