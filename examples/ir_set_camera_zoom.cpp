#include "stdio.h"
#include <pthread.h>
#include <cstdlib>

#include"payloadSdkInterface.h"

#if (CONTROL_METHOD == CONTROL_UART)
T_ConnInfo s_conn = {
    CONTROL_UART,
    payload_uart_port,
    payload_uart_baud
};
#else
T_ConnInfo s_conn = {
    CONTROL_UDP,
    udp_ip_target,
    udp_port_target
};
#endif

PayloadSdkInterface* my_payload = nullptr;

void quit_handler(int sig);

int main(int argc, char *argv[]){
	printf("Starting CaptureImage example...\n");
	signal(SIGINT,quit_handler);

	// create payloadsdk object
	my_payload = new PayloadSdkInterface(s_conn);

	// init payload
	my_payload->sdkInitConnection();
	printf("Waiting for payload signal! \n");

	my_payload->checkPayloadConnection();

	// set view source
    printf("Set view source to IR! \n");
	my_payload->setPayloadCameraParam(PAYLOAD_CAMERA_VIEW_SRC, PAYLOAD_CAMERA_VIEW_IR, PARAM_TYPE_UINT32);
    usleep(1000000);

    printf("Set zoom level to 1x ! \n");
	my_payload->setPayloadCameraParam(PAYLOAD_CAMERA_IR_ZOOM_FACTOR, ZOOM_IR_1X, PARAM_TYPE_UINT32);
    usleep(3000000);

    // zoom step
    printf("Zoom In 4 times! \n");
    my_payload->setCameraZoom(ZOOM_TYPE_STEP, ZOOM_IN); // zoom in
    usleep(1000000); // sleep 1s
    my_payload->setCameraZoom(ZOOM_TYPE_STEP, ZOOM_IN); // zoom in
    usleep(1000000); // sleep 1s
    my_payload->setCameraZoom(ZOOM_TYPE_STEP, ZOOM_IN); // zoom in
    usleep(1000000); // sleep 1s
    my_payload->setCameraZoom(ZOOM_TYPE_STEP, ZOOM_IN); // zoom in
    usleep(1000000); // sleep 1s
    printf("Zoom Out 2 times! \n");
    my_payload->setCameraZoom(ZOOM_TYPE_STEP, ZOOM_OUT); // zoom out
    usleep(1000000); // sleep 1s
    my_payload->setCameraZoom(ZOOM_TYPE_STEP, ZOOM_OUT); // zoom out
    usleep(1000000); // sleep 1s

    // zoom continuous
    printf("Start Zoom In! \n");
    my_payload->setCameraZoom(ZOOM_TYPE_CONTINUOUS, ZOOM_IN); // zoom in
    usleep(5000000); // sleep 5s
    printf("Stop Zoom! \n");
    my_payload->setCameraZoom(ZOOM_TYPE_CONTINUOUS, ZOOM_STOP); // stop zoom
    usleep(2000000); // sleep 2s
    printf("Start Zoom Out! \n");
    my_payload->setCameraZoom(ZOOM_TYPE_CONTINUOUS, ZOOM_OUT); // zoom out
    usleep(7000000); // sleep 7s
    printf("Stop Zoom! \n");
    my_payload->setCameraZoom(ZOOM_TYPE_CONTINUOUS, ZOOM_STOP); // stop zoom
    usleep(2000000); // sleep 2s

    // zoom range
    printf("Zoom Range 50%! \n");
    my_payload->setCameraZoom(ZOOM_TYPE_RANGE, 50.0); // zoom 50%
    usleep(3000000); // sleep 3s
    printf("Zoom Range 70%! \n");
    my_payload->setCameraZoom(ZOOM_TYPE_RANGE, 70.0); // zoom 70%
    usleep(3000000); // sleep 3s
    printf("Zoom Range 100%! \n");
    my_payload->setCameraZoom(ZOOM_TYPE_RANGE, 100.0); // zoom 100%
    usleep(3000000); // sleep 3s
    printf("Zoom Range 0%! \n");
    my_payload->setCameraZoom(ZOOM_TYPE_RANGE, 0.0);  // zoom 0%
    usleep(5000000); // sleep 5s
    printf("!--------------------! \n");

    // close payload interface
    try {
        my_payload->sdkQuit();
    }
    catch (int error){}
        
	return 0;
}

void quit_handler( int sig ){
    printf("\n");
    printf("TERMINATING AT USER REQUEST \n");
    printf("\n");

    // close payload interface
    try {
        my_payload->sdkQuit();
    }
    catch (int error){}

    // end program here
    exit(0);
}
