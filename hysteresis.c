#include <stdio.h>
#define NUM_LEVEL 4
#define HYS_SEN 2.5
#define STEP 100/NUM_LEVEL

unsigned int hysteresis(unsigned int input_percent){
    
    //static prev_value =0;
    int encoded_step = 0;
    int range        = 0;
    
    // steps loop 0-25,25-50,50-75,75-100
    for(int steps=0; steps<100; steps=range)
    {  
        range = steps + STEP;
        float middlerange= (steps + range)/2.0;
        printf("i=%d and y-%d,middle=%f\n",steps,range,middlerange);
        
            if(input_percent > range)
            {
                encoded_step++;
                continue;
            }
            
            if((middlerange - HYS_SEN) <= input_percent)
            {
              encoded_step++;
            }
            
            return encoded_step;
    }
    
}
int main()
{
   printf(" number of step=%d\n",hysteresis(10));
    return 0;
}
