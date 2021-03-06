// Baseado em: https://bitbucket.org/cinqlair/mpu9250/src/0b38d94e630291eeff31fb0c1897425f64cb0c31/mpu9250_OpenGL/?at=master


//#include <QtGui/QApplication>
#include <QtWidgets/QApplication>
#include "mainwindow.h"
#include <carmen/carmen.h>
#include <carmen/xsens_mtig_interface.h>
#include <carmen/xsens_interface.h>
#include <carmen/xsens_messages.h>
#include <carmen/rotation_geometry.h>
#include <carmen/gps_xyz_interface.h>
carmen_xsens_global_quat_message *xsens_quat_message, *data;
carmen_xsens_global_message *data_pose, *pose;


rotation_matrix *
create_rotation_matrix_from_quaternions_new(carmen_xsens_quat quat)
{
	double q0 = quat.m_data[0];
	double q1 = quat.m_data[1];
	double q2 = quat.m_data[2];
	double q3 = quat.m_data[3];

	rotation_matrix *r_matrix =(rotation_matrix *) malloc(sizeof(rotation_matrix));

	r_matrix->matrix[0 + 3 * 0] = q0 * q0 + q1*q1 - q2*q2 - q3*q3;
	r_matrix->matrix[0 + 3 * 1] = 2*(q1*q2 - q0*q3);
	r_matrix->matrix[0 + 3*2] = 2*(q0*q2 + q1*q3);
	r_matrix->matrix[1 + 3*0] =	2*(q1*q2 + q0*q3);
	r_matrix->matrix[1 + 3*1] = q0*q0 - q1*q1 + q2*q2 - q3*q3;
	r_matrix->matrix[1 + 3*2] = 2*(q2*q3 - q0*q1);
	r_matrix->matrix[2 + 3*0] = 2*(q1*q3 - q0*q2);
	r_matrix->matrix[2 + 3*1] = 2*(q0*q1 + q2*q3);
	r_matrix->matrix[2 + 3*2] = q0*q0 - q1*q1 - q2*q2 + q3*q3;

	return (r_matrix);
}


carmen_orientation_3D_t
get_angles_from_rotation_matrix_new(rotation_matrix *r_matrix)//, carmen_xsens_global_quat_message *xsens_quat_message)
{
	carmen_orientation_3D_t angles;

	angles.roll = atan2(r_matrix->matrix[2 + 3*1], r_matrix->matrix[2 + 3*2]);
	angles.pitch = -asin(r_matrix->matrix[2 + 3*0]);
	angles.yaw = -atan2(r_matrix->matrix[1 + 3*0], r_matrix->matrix[0 + 3*0]); // O yaw tem que ficar ao contrario por algum sinal trocado codigo adentro (OpenGL?)

//	printf("r %+03.1lf   p %+03.1lf   y %+03.1lf   **  x %+03.1lf   y %+03.1lf   z %+03.1lf\n",
//			carmen_radians_to_degrees(angles.roll),
//			carmen_radians_to_degrees(angles.pitch),
//			carmen_radians_to_degrees(angles.yaw),
//			xsens_quat_message->m_acc.x,
//			xsens_quat_message->m_acc.y,
//			xsens_quat_message->m_acc.z);

	return (angles);
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//																								//
// Publishers																					//
//																								//
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////////////////
//																								//
// Handlers																						//
//																								//
//////////////////////////////////////////////////////////////////////////////////////////////////

void
xsens_quat_message_handler(carmen_xsens_global_quat_message *xsens_quat_message)
{
	data->m_acc.x = xsens_quat_message->m_acc.x;
	data->m_acc.y = xsens_quat_message->m_acc.y;
	data->m_acc.z = xsens_quat_message->m_acc.z;

	data->m_gyr.x = xsens_quat_message->m_gyr.x;
	data->m_gyr.y = xsens_quat_message->m_gyr.y;
	data->m_gyr.z = xsens_quat_message->m_gyr.z;


	data->m_mag.x = xsens_quat_message->m_mag.x;
	data->m_mag.y = xsens_quat_message->m_mag.y;
	data->m_mag.z = xsens_quat_message->m_mag.z;

	printf ("%lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
			xsens_quat_message->m_acc.x, xsens_quat_message->m_acc.y, xsens_quat_message->m_acc.z,
			xsens_quat_message->m_gyr.x, xsens_quat_message->m_gyr.y, xsens_quat_message->m_gyr.z,
			xsens_quat_message->m_mag.x, xsens_quat_message->m_mag.y, xsens_quat_message->m_mag.z);

	data->quat_data = xsens_quat_message->quat_data;

	rotation_matrix *r_mat = create_rotation_matrix_from_quaternions_new(data->quat_data);
	carmen_orientation_3D_t euler_angles = get_angles_from_rotation_matrix_new(r_mat);//, xsens_quat_message);
	destroy_rotation_matrix(r_mat);

	data_pose->m_roll = euler_angles.roll;
	data_pose->m_pitch = euler_angles.pitch;
	data_pose->m_yaw = euler_angles.yaw;
}


void
xsens_global_euler_message_handler(carmen_xsens_global_euler_message *xsens_global_euler_message)
{
	data_pose->m_roll = carmen_degrees_to_radians(xsens_global_euler_message->euler_data.m_roll);
	data_pose->m_pitch = carmen_degrees_to_radians(xsens_global_euler_message->euler_data.m_pitch);
	data_pose->m_yaw = carmen_degrees_to_radians(xsens_global_euler_message->euler_data.m_yaw);
}


void
shutdown_module(int signo)
{
    if (signo == SIGINT)
    {
        carmen_ipc_disconnect();

        printf("Imu Interface: disconnected.\n");
        exit(0);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
//																								//
// Inicializations																				//
//																								//
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////


int 
main(int argc, char *argv[])
{
	carmen_ipc_initialize(argc, argv);
	carmen_param_check_version(argv[0]);
	signal(SIGINT, shutdown_module);

	data = (carmen_xsens_global_quat_message*) calloc (1, sizeof (carmen_xsens_global_quat_message));
	data_pose = (carmen_xsens_global_message*) calloc(1, sizeof (carmen_xsens_global_message));

	carmen_xsens_subscribe_xsens_global_quat_message(NULL,
			(carmen_handler_t) xsens_quat_message_handler, CARMEN_SUBSCRIBE_LATEST);
//	carmen_xsens_subscribe_xsens_global_euler_message(NULL,
//			(carmen_handler_t) xsens_global_euler_message_handler, CARMEN_SUBSCRIBE_LATEST);

	QApplication a(argc, argv);
    MainWindow w(0,800,600, data, data_pose);

    w.show();

    return (a.exec());
}


