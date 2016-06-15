LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= sensor_ctrl.c

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_MODULE:= vivo_cam_sensor
include $(BUILD_EXECUTABLE)
