/* date = January 6th 2021 0:51 am */
#ifndef FLIGHTCONTROL_HELPERS_H
#define FLIGHTCONTROL_HELPERS_H

void Helpers_Display_Matrix4(mat4 matrix, u8 *string)
{
    printf("%s \n", string);
    printf("| %f %f %f %f | \n", matrix[0][0], matrix[1][0], matrix[2][0], matrix[3][0]);
    printf("| %f %f %f %f | \n", matrix[0][1], matrix[1][1], matrix[2][1], matrix[3][1]);
    printf("| %f %f %f %f | \n", matrix[0][2], matrix[1][2], matrix[2][2], matrix[3][2]);
    printf("| %f %f %f %f | \n", matrix[0][3], matrix[1][3], matrix[2][3], matrix[3][3]);
    
    return;
}

void Helpers_Display_Matrix3(mat3 matrix, u8 *string)
{
    printf("%s \n", string);
    printf("| %f %f %f | \n", matrix[0][0], matrix[1][0], matrix[2][0]);
    printf("| %f %f %f | \n", matrix[0][1], matrix[1][1], matrix[2][1]);
    printf("| %f %f %f | \n", matrix[0][2], matrix[1][2], matrix[2][2]);
    
    return;
}


#endif //FLIGHTCONTROL_HELPERS_H
