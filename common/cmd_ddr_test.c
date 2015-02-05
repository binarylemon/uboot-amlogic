#include <common.h>

#include <asm/io.h>
#include <asm/arch/io.h>
#include <asm/arch/register.h>

#define TDATA32F 0xffffffff
#define TDATA32A 0xaaaaaaaa
#define TDATA325 0x55555555

static void ddr_write(void *buff, unsigned m_length)
{
    unsigned *p;
    unsigned i, j, n;
    unsigned m_len = m_length;

    p = (unsigned *)buff;

    while(m_len)
    {
        for(j=0;j<32;j++)
        {
            if(m_len >= 128)
                n = 32;
            else
                n = m_len>>2;

            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                    case 0:
                    case 9:
                    case 14:
                    case 25:
                    case 30:
                        *(p+i) = TDATA32F;
                        break;
                    case 1:
                    case 6:
                    case 8:
                    case 17:
                    case 22:
                        *(p+i) = 0;
                        break;
                    case 16:
                    case 23:
                    case 31:
                        *(p+i) = TDATA32A;
                        break;
                    case 7:
                    case 15:
                    case 24:
                        *(p+i) = TDATA325;
                        break;
                    case 2:
                    case 4:
                    case 10:
                    case 12:
                    case 19:
                    case 21:
                    case 27:
                    case 29:
                        *(p+i) = 1<<j;
                        break;
                    case 3:
                    case 5:
                    case 11:
                    case 13:
                    case 18:
                    case 20:
                    case 26:
                    case 28:
                        *(p+i) = ~(1<<j);
                        break;
                }
            }

            if(m_len > 128)
            {
                m_len -= 128;
                p += 32;
            }
            else
            {
                p += (m_len>>2);
                m_len = 0;
                break;
            }
        }
    }
}





static void ddr_read(void *buff, unsigned m_length)
{
    unsigned *p;
    unsigned i, j, n;
    unsigned m_len = m_length;

    p = (unsigned *)buff;

    while(m_len)
    {
        for(j=0;j<32;j++)
        {
            if(m_len >= 128)
                n = 32;
            else
                n = m_len>>2;

            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                    case 0:
                    case 9:
                    case 14:
                    case 25:
                    case 30:
                        if(*(p+i) != TDATA32F)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), TDATA32F);
                        break;
                    case 1:
                    case 6:
                    case 8:
                    case 17:
                    case 22:
                        if(*(p+i) != 0)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0);
                        break;
                    case 16:
                    case 23:
                    case 31:
                        if(*(p+i) != TDATA32A)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), TDATA32A);
                        break;
                    case 7:
                    case 15:
                    case 24:
                        if(*(p+i) != TDATA325)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), TDATA325);
                        break;
                    case 2:
                    case 4:
                    case 10:
                    case 12:
                    case 19:
                    case 21:
                    case 27:
                    case 29:
                        if(*(p+i) != 1<<j)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 1<<j);
                        break;
                    case 3:
                    case 5:
                    case 11:
                    case 13:
                    case 18:
                    case 20:
                    case 26:
                    case 28:
                        if(*(p+i) != ~(1<<j))
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~(1<<j));
                        break;
                }
            }

            if(m_len > 128)
            {
                m_len -= 128;
                p += 32;
            }
            else
            {
                p += (m_len>>2);
                m_len = 0;
                break;
            }
        }
    }
}

///*
#define DDR_PATTERN_LOOP_1 32
#define DDR_PATTERN_LOOP_2 64
#define DDR_PATTERN_LOOP_3 96
static void ddr_write_pattern4_cross_talk_p(void *buff, unsigned m_length)
{
    unsigned *p;
 //   unsigned i, j, n;
	 unsigned i, n;
    unsigned m_len = m_length;
//#define ddr_pattern_loop 32
    p = (unsigned *)buff;

    while(m_len)
    {
      //  for(j=0;j<32;j++)
        {
            if(m_len >= 128*4)
                n = 32*4;
            else
                n = m_len>>2;

            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 8:
                    case 9:
                    case 10:
                    case 11:
		      case 16:
                    case 17:
                    case 18:
                    case 19:
                    case 24:
                    case 25:
                    case 26:
                    case 27:
                 //   case 30:
                        *(p+i) = TDATA32F;
                        break;
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
		      case 20:
                    case 21:
                    case 22:
                    case 23:
		      case 28:
                    case 29:
                    case 30:
                    case 31:
                 //   case 22:
                        *(p+i) = 0;
                        break;
		      case DDR_PATTERN_LOOP_1+0:
                    case DDR_PATTERN_LOOP_1+1:
                    case DDR_PATTERN_LOOP_1+2:
                    case DDR_PATTERN_LOOP_1+3:
                    case DDR_PATTERN_LOOP_1+8:
                    case DDR_PATTERN_LOOP_1+9:
                    case DDR_PATTERN_LOOP_1+10:
                    case DDR_PATTERN_LOOP_1+11:
		      case DDR_PATTERN_LOOP_1+16:
                    case DDR_PATTERN_LOOP_1+17:
                    case DDR_PATTERN_LOOP_1+18:
                    case DDR_PATTERN_LOOP_1+19:
                    case DDR_PATTERN_LOOP_1+24:
                    case DDR_PATTERN_LOOP_1+25:
                    case DDR_PATTERN_LOOP_1+26:
                    case DDR_PATTERN_LOOP_1+27:
                 //   case 30:
                          *(p+i) = TDATA32A;
                        break;
                    case DDR_PATTERN_LOOP_1+4:
                    case DDR_PATTERN_LOOP_1+5:
                    case DDR_PATTERN_LOOP_1+6:
                    case DDR_PATTERN_LOOP_1+7:
                    case DDR_PATTERN_LOOP_1+12:
                    case DDR_PATTERN_LOOP_1+13:
                    case DDR_PATTERN_LOOP_1+14:
                    case DDR_PATTERN_LOOP_1+15:
		      case DDR_PATTERN_LOOP_1+20:
                    case DDR_PATTERN_LOOP_1+21:
                    case DDR_PATTERN_LOOP_1+22:
                    case DDR_PATTERN_LOOP_1+23:
		      case DDR_PATTERN_LOOP_1+28:
                    case DDR_PATTERN_LOOP_1+29:
                    case DDR_PATTERN_LOOP_1+30:
                    case DDR_PATTERN_LOOP_1+31:
				*(p+i) = TDATA325;		
                   
                      
                        break;
               case DDR_PATTERN_LOOP_2+0:
		 case DDR_PATTERN_LOOP_2+1:
		 case DDR_PATTERN_LOOP_2+2:
		 case DDR_PATTERN_LOOP_2+3:
			   	*(p+i) =0xfe01fe01;
				  break;
               case DDR_PATTERN_LOOP_2+4:
		 case DDR_PATTERN_LOOP_2+5:
		 case DDR_PATTERN_LOOP_2+6:
		 case DDR_PATTERN_LOOP_2+7:
			   	*(p+i) =0xfd02fd02;
				  break;
               case DDR_PATTERN_LOOP_2+8:
		 case DDR_PATTERN_LOOP_2+9:
		 case DDR_PATTERN_LOOP_2+10:
		 case DDR_PATTERN_LOOP_2+11:
			   	*(p+i) =0xfb04fb04;
				  break;
               case DDR_PATTERN_LOOP_2+12:
		 case DDR_PATTERN_LOOP_2+13:
		 case DDR_PATTERN_LOOP_2+14:
		 case DDR_PATTERN_LOOP_2+15:
			   	*(p+i) =0xf708f708;	
				  break;
               case DDR_PATTERN_LOOP_2+16:
		 case DDR_PATTERN_LOOP_2+17:
		 case DDR_PATTERN_LOOP_2+18:
		 case DDR_PATTERN_LOOP_2+19:
			   	*(p+i) =0xef10ef10;	
				  break;
               case DDR_PATTERN_LOOP_2+20:
		 case DDR_PATTERN_LOOP_2+21:
		 case DDR_PATTERN_LOOP_2+22:
		 case DDR_PATTERN_LOOP_2+23:
			   	*(p+i) =0xdf20df20;	
				  break;
               case DDR_PATTERN_LOOP_2+24:
		 case DDR_PATTERN_LOOP_2+25:
		 case DDR_PATTERN_LOOP_2+26:
		 case DDR_PATTERN_LOOP_2+27:
			   	*(p+i) =0xbf40bf40;
				  break;
               case DDR_PATTERN_LOOP_2+28:
		 case DDR_PATTERN_LOOP_2+29:
		 case DDR_PATTERN_LOOP_2+30:
		 case DDR_PATTERN_LOOP_2+31:
			   	*(p+i) =0x7f807f80;	
				  break;
               case DDR_PATTERN_LOOP_3+0:
		 case DDR_PATTERN_LOOP_3+1:
		 case DDR_PATTERN_LOOP_3+2:
		 case DDR_PATTERN_LOOP_3+3:
			   	*(p+i) =0x00000100;
				  break;
               case DDR_PATTERN_LOOP_3+4:
		 case DDR_PATTERN_LOOP_3+5:
		 case DDR_PATTERN_LOOP_3+6:
		 case DDR_PATTERN_LOOP_3+7:
			   	*(p+i) =0x00000200;
				  break;
               case DDR_PATTERN_LOOP_3+8:
		 case DDR_PATTERN_LOOP_3+9:
		 case DDR_PATTERN_LOOP_3+10:
		 case DDR_PATTERN_LOOP_3+11:
			   	*(p+i) =0x00000400;
				  break;
               case DDR_PATTERN_LOOP_3+12:
		 case DDR_PATTERN_LOOP_3+13:
		 case DDR_PATTERN_LOOP_3+14:
		 case DDR_PATTERN_LOOP_3+15:
			   	*(p+i) =0x00000800;	
				  break;
               case DDR_PATTERN_LOOP_3+16:
		 case DDR_PATTERN_LOOP_3+17:
		 case DDR_PATTERN_LOOP_3+18:
		 case DDR_PATTERN_LOOP_3+19:
			   	*(p+i) =0x00001000;	
				  break;
               case DDR_PATTERN_LOOP_3+20:
		 case DDR_PATTERN_LOOP_3+21:
		 case DDR_PATTERN_LOOP_3+22:
		 case DDR_PATTERN_LOOP_3+23:
			   	*(p+i) =0x00002000;	
				  break;
               case DDR_PATTERN_LOOP_3+24:
		 case DDR_PATTERN_LOOP_3+25:
		 case DDR_PATTERN_LOOP_3+26:
		 case DDR_PATTERN_LOOP_3+27:
			   	*(p+i) =0x00004000;
				  break;
               case DDR_PATTERN_LOOP_3+28:
		 case DDR_PATTERN_LOOP_3+29:
		 case DDR_PATTERN_LOOP_3+30:
		 case DDR_PATTERN_LOOP_3+31:
			   	*(p+i) =0x00008000;	
				  break;
                       
                      
                }
            }

            if(m_len >( 128*4))
            {
                m_len -=( 128*4);
                p += 32*4;
            }
            else
            {
                p += (m_len>>2);
                m_len = 0;
                break;
            }
        }
    }
}


static void ddr_read_pattern4_cross_talk_p(void *buff, unsigned m_length)
{
    unsigned *p;
  //  unsigned i, j, n;
	 unsigned i, n;
    unsigned m_len = m_length;

    p = (unsigned *)buff;

    while(m_len)
    {
      //  for(j=0;j<32;j++)
        {
            if(m_len >= 128*4)
                n = 32*4;
            else
                n = m_len>>2;

            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                     case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 8:
                    case 9:
                    case 10:
                    case 11:
		      case 16:
                    case 17:
                    case 18:
                    case 19:
                    case 24:
                    case 25:
                    case 26:
                    case 27:
                 //   case 30:
                  //      *(p+i) = TDATA32F;
                        if(*(p+i) != TDATA32F)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), TDATA32F);
                        break;
                     case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
		      case 20:
                    case 21:
                    case 22:
                    case 23:
		      case 28:
                    case 29:
                    case 30:
                    case 31:
                 //   case 22:
                     //   *(p+i) = 0;
                        if(*(p+i) != 0)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0);
                        break;
                    case DDR_PATTERN_LOOP_1+0:
                    case DDR_PATTERN_LOOP_1+1:
                    case DDR_PATTERN_LOOP_1+2:
                    case DDR_PATTERN_LOOP_1+3:
                    case DDR_PATTERN_LOOP_1+8:
                    case DDR_PATTERN_LOOP_1+9:
                    case DDR_PATTERN_LOOP_1+10:
                    case DDR_PATTERN_LOOP_1+11:
		      case DDR_PATTERN_LOOP_1+16:
                    case DDR_PATTERN_LOOP_1+17:
                    case DDR_PATTERN_LOOP_1+18:
                    case DDR_PATTERN_LOOP_1+19:
                    case DDR_PATTERN_LOOP_1+24:
                    case DDR_PATTERN_LOOP_1+25:
                    case DDR_PATTERN_LOOP_1+26:
                    case DDR_PATTERN_LOOP_1+27:
                 //   case 30:
                  //        *(p+i) = TDATA32A;
                        if(*(p+i) != TDATA32A)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), TDATA32A);
                        break;
                       case DDR_PATTERN_LOOP_1+4:
                    case DDR_PATTERN_LOOP_1+5:
                    case DDR_PATTERN_LOOP_1+6:
                    case DDR_PATTERN_LOOP_1+7:
                    case DDR_PATTERN_LOOP_1+12:
                    case DDR_PATTERN_LOOP_1+13:
                    case DDR_PATTERN_LOOP_1+14:
                    case DDR_PATTERN_LOOP_1+15:
		      case DDR_PATTERN_LOOP_1+20:
                    case DDR_PATTERN_LOOP_1+21:
                    case DDR_PATTERN_LOOP_1+22:
                    case DDR_PATTERN_LOOP_1+23:
		      case DDR_PATTERN_LOOP_1+28:
                    case DDR_PATTERN_LOOP_1+29:
                    case DDR_PATTERN_LOOP_1+30:
                    case DDR_PATTERN_LOOP_1+31:
			//	*(p+i) = TDATA325;	
                        if(*(p+i) != TDATA325)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), TDATA325);
                        break;
                     case DDR_PATTERN_LOOP_2+0:
		 case DDR_PATTERN_LOOP_2+1:
		 case DDR_PATTERN_LOOP_2+2:
		 case DDR_PATTERN_LOOP_2+3:
			//   	*(p+i) =0xfe01fe01;
                        if(*(p+i) !=0xfe01fe01)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xfe01fe01);
                        break;
                    case DDR_PATTERN_LOOP_2+4:
		 case DDR_PATTERN_LOOP_2+5:
		 case DDR_PATTERN_LOOP_2+6:
		 case DDR_PATTERN_LOOP_2+7:
			  // 	*(p+i) =0xfd02fd02;
                        if(*(p+i) != 0xfd02fd02)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xfd02fd02);
                        break;

               case DDR_PATTERN_LOOP_2+8:
		 case DDR_PATTERN_LOOP_2+9:
		 case DDR_PATTERN_LOOP_2+10:
		 case DDR_PATTERN_LOOP_2+11:
			   //	*(p+i) =0xfb04fb04;
			     if(*(p+i) != 0xfb04fb04)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xfb04fb04);
				  break;
               case DDR_PATTERN_LOOP_2+12:
		 case DDR_PATTERN_LOOP_2+13:
		 case DDR_PATTERN_LOOP_2+14:
		 case DDR_PATTERN_LOOP_2+15:
			   //	*(p+i) =0xf7b08f708;	
				  if(*(p+i) != 0xf708f708)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xf708f708);
				  break;
               case DDR_PATTERN_LOOP_2+16:
		 case DDR_PATTERN_LOOP_2+17:
		 case DDR_PATTERN_LOOP_2+18:
		 case DDR_PATTERN_LOOP_2+19:
			   //	*(p+i) =0xef10ef10;	
				  if(*(p+i) != 0xef10ef10)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xef10ef10);
				  break;
               case DDR_PATTERN_LOOP_2+20:
		 case DDR_PATTERN_LOOP_2+21:
		 case DDR_PATTERN_LOOP_2+22:
		 case DDR_PATTERN_LOOP_2+23:
			   //	*(p+i) =0xdf20df20;	
				  if(*(p+i) != 0xdf20df20)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xdf20df20);
				  break;
               case DDR_PATTERN_LOOP_2+24:
		 case DDR_PATTERN_LOOP_2+25:
		 case DDR_PATTERN_LOOP_2+26:
		 case DDR_PATTERN_LOOP_2+27:
			 //  	*(p+i) =0xbf40bf40;
				  if(*(p+i) != 0xbf40bf40)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xbf40bf40);
				  break;
               case DDR_PATTERN_LOOP_2+28:
		 case DDR_PATTERN_LOOP_2+29:
		 case DDR_PATTERN_LOOP_2+30:
		 case DDR_PATTERN_LOOP_2+31:
			//   	*(p+i) =0x7f807f80;	
				  if(*(p+i) != 0x7f807f80)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0x7f807f80);
				  break;
				  break;
               case DDR_PATTERN_LOOP_3+0:
		 case DDR_PATTERN_LOOP_3+1:
		 case DDR_PATTERN_LOOP_3+2:
		 case DDR_PATTERN_LOOP_3+3:
			   //	*(p+i) =0x00000100;
					  if(*(p+i) != 0x00000100)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0x00000100);
				  break;
               case DDR_PATTERN_LOOP_3+4:
		 case DDR_PATTERN_LOOP_3+5:
		 case DDR_PATTERN_LOOP_3+6:
		 case DDR_PATTERN_LOOP_3+7:
			 //  	*(p+i) =0x00000100;
				  if(*(p+i) != 0x00000200)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0x00000200);
				  break;
               case DDR_PATTERN_LOOP_3+8:
		 case DDR_PATTERN_LOOP_3+9:
		 case DDR_PATTERN_LOOP_3+10:
		 case DDR_PATTERN_LOOP_3+11:
			   //	*(p+i) =0x00000100;
				 if(*(p+i) != 0x00000400)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0x00000400);
				  break;
               case DDR_PATTERN_LOOP_3+12:
		 case DDR_PATTERN_LOOP_3+13:
		 case DDR_PATTERN_LOOP_3+14:
		 case DDR_PATTERN_LOOP_3+15:
			   //	*(p+i) =0x00000100;	
				 if(*(p+i) != 0x00000800)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0x00000800);
				  break;
               case DDR_PATTERN_LOOP_3+16:
		 case DDR_PATTERN_LOOP_3+17:
		 case DDR_PATTERN_LOOP_3+18:
		 case DDR_PATTERN_LOOP_3+19:
			   //	*(p+i) =0xfffffeff;	
				 if(*(p+i) != 0x00001000)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0x00001000);
				  break;
               case DDR_PATTERN_LOOP_3+20:
		 case DDR_PATTERN_LOOP_3+21:
		 case DDR_PATTERN_LOOP_3+22:
		 case DDR_PATTERN_LOOP_3+23:
			  // 	*(p+i) =0xfffffeff;	
				 if(*(p+i) != 0x00002000)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0x00002000);
				  break;
               case DDR_PATTERN_LOOP_3+24:
		 case DDR_PATTERN_LOOP_3+25:
		 case DDR_PATTERN_LOOP_3+26:
		 case DDR_PATTERN_LOOP_3+27:
			   //	*(p+i) =0xfffffeff;
				 if(*(p+i) != 0x00004000)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0x00004000);
				  break;
               case DDR_PATTERN_LOOP_3+28:
		 case DDR_PATTERN_LOOP_3+29:
		 case DDR_PATTERN_LOOP_3+30:
		 case DDR_PATTERN_LOOP_3+31:
			   //	*(p+i) =0xfffffeff;	
				 if(*(p+i) != 0x00008000)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0x00008000);
				  break;
                       

						
                }
            }

            if(m_len > 128*4)
            {
                m_len -= 128*4;
                p += 32*4;
            }
            else
            {
                p += (m_len>>2);
                m_len = 0;
                break;
            }
        }
    }
}
//*/
static void ddr_write_pattern4_cross_talk_n(void *buff, unsigned m_length)
{
    unsigned *p;
 //   unsigned i, j, n;
	 unsigned i, n;
    unsigned m_len = m_length;
//#define ddr_pattern_loop 32
    p = (unsigned *)buff;

    while(m_len)
    {
      //  for(j=0;j<32;j++)
        {
            if(m_len >= 128*4)
                n = 32*4;
            else
                n = m_len>>2;

            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 8:
                    case 9:
                    case 10:
                    case 11:
		      case 16:
                    case 17:
                    case 18:
                    case 19:
                    case 24:
                    case 25:
                    case 26:
                    case 27:
                 //   case 30:
                        *(p+i) = ~TDATA32F;
                        break;
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
		      case 20:
                    case 21:
                    case 22:
                    case 23:
		      case 28:
                    case 29:
                    case 30:
                    case 31:
                 //   case 22:
                        *(p+i) = ~0;
                        break;
		      case DDR_PATTERN_LOOP_1+0:
                    case DDR_PATTERN_LOOP_1+1:
                    case DDR_PATTERN_LOOP_1+2:
                    case DDR_PATTERN_LOOP_1+3:
                    case DDR_PATTERN_LOOP_1+8:
                    case DDR_PATTERN_LOOP_1+9:
                    case DDR_PATTERN_LOOP_1+10:
                    case DDR_PATTERN_LOOP_1+11:
		      case DDR_PATTERN_LOOP_1+16:
                    case DDR_PATTERN_LOOP_1+17:
                    case DDR_PATTERN_LOOP_1+18:
                    case DDR_PATTERN_LOOP_1+19:
                    case DDR_PATTERN_LOOP_1+24:
                    case DDR_PATTERN_LOOP_1+25:
                    case DDR_PATTERN_LOOP_1+26:
                    case DDR_PATTERN_LOOP_1+27:
                 //   case 30:
                          *(p+i) = ~TDATA32A;
                        break;
                    case DDR_PATTERN_LOOP_1+4:
                    case DDR_PATTERN_LOOP_1+5:
                    case DDR_PATTERN_LOOP_1+6:
                    case DDR_PATTERN_LOOP_1+7:
                    case DDR_PATTERN_LOOP_1+12:
                    case DDR_PATTERN_LOOP_1+13:
                    case DDR_PATTERN_LOOP_1+14:
                    case DDR_PATTERN_LOOP_1+15:
		      case DDR_PATTERN_LOOP_1+20:
                    case DDR_PATTERN_LOOP_1+21:
                    case DDR_PATTERN_LOOP_1+22:
                    case DDR_PATTERN_LOOP_1+23:
		      case DDR_PATTERN_LOOP_1+28:
                    case DDR_PATTERN_LOOP_1+29:
                    case DDR_PATTERN_LOOP_1+30:
                    case DDR_PATTERN_LOOP_1+31:
				*(p+i) =~TDATA325;		
                   
                      
                        break;
               case DDR_PATTERN_LOOP_2+0:
		 case DDR_PATTERN_LOOP_2+1:
		 case DDR_PATTERN_LOOP_2+2:
		 case DDR_PATTERN_LOOP_2+3:
			   	*(p+i) =~0xfe01fe01;
				  break;
               case DDR_PATTERN_LOOP_2+4:
		 case DDR_PATTERN_LOOP_2+5:
		 case DDR_PATTERN_LOOP_2+6:
		 case DDR_PATTERN_LOOP_2+7:
			   	*(p+i) =~0xfd02fd02;
				  break;
               case DDR_PATTERN_LOOP_2+8:
		 case DDR_PATTERN_LOOP_2+9:
		 case DDR_PATTERN_LOOP_2+10:
		 case DDR_PATTERN_LOOP_2+11:
			   	*(p+i) =~0xfb04fb04;
				  break;
               case DDR_PATTERN_LOOP_2+12:
		 case DDR_PATTERN_LOOP_2+13:
		 case DDR_PATTERN_LOOP_2+14:
		 case DDR_PATTERN_LOOP_2+15:
			   	*(p+i) =~0xf708f708;	
				  break;
               case DDR_PATTERN_LOOP_2+16:
		 case DDR_PATTERN_LOOP_2+17:
		 case DDR_PATTERN_LOOP_2+18:
		 case DDR_PATTERN_LOOP_2+19:
			   	*(p+i) =~0xef10ef10;	
				  break;
               case DDR_PATTERN_LOOP_2+20:
		 case DDR_PATTERN_LOOP_2+21:
		 case DDR_PATTERN_LOOP_2+22:
		 case DDR_PATTERN_LOOP_2+23:
			   	*(p+i) =~0xdf20df20;	
				  break;
               case DDR_PATTERN_LOOP_2+24:
		 case DDR_PATTERN_LOOP_2+25:
		 case DDR_PATTERN_LOOP_2+26:
		 case DDR_PATTERN_LOOP_2+27:
			   	*(p+i) =~0xbf40bf40;
				  break;
               case DDR_PATTERN_LOOP_2+28:
		 case DDR_PATTERN_LOOP_2+29:
		 case DDR_PATTERN_LOOP_2+30:
		 case DDR_PATTERN_LOOP_2+31:
			   	*(p+i) =~0x7f807f80;	
				  break;
               case DDR_PATTERN_LOOP_3+0:
		 case DDR_PATTERN_LOOP_3+1:
		 case DDR_PATTERN_LOOP_3+2:
		 case DDR_PATTERN_LOOP_3+3:
			   	*(p+i) =~0x00000100;
				  break;
               case DDR_PATTERN_LOOP_3+4:
		 case DDR_PATTERN_LOOP_3+5:
		 case DDR_PATTERN_LOOP_3+6:
		 case DDR_PATTERN_LOOP_3+7:
			   	*(p+i) =~0x00000200;
				  break;
               case DDR_PATTERN_LOOP_3+8:
		 case DDR_PATTERN_LOOP_3+9:
		 case DDR_PATTERN_LOOP_3+10:
		 case DDR_PATTERN_LOOP_3+11:
			   	*(p+i) =~0x00000400;
				  break;
               case DDR_PATTERN_LOOP_3+12:
		 case DDR_PATTERN_LOOP_3+13:
		 case DDR_PATTERN_LOOP_3+14:
		 case DDR_PATTERN_LOOP_3+15:
			   	*(p+i) =~0x00000800;	
				  break;
               case DDR_PATTERN_LOOP_3+16:
		 case DDR_PATTERN_LOOP_3+17:
		 case DDR_PATTERN_LOOP_3+18:
		 case DDR_PATTERN_LOOP_3+19:
			   	*(p+i) =~0x00001000;	
				  break;
               case DDR_PATTERN_LOOP_3+20:
		 case DDR_PATTERN_LOOP_3+21:
		 case DDR_PATTERN_LOOP_3+22:
		 case DDR_PATTERN_LOOP_3+23:
			   	*(p+i) =~0x00002000;	
				  break;
               case DDR_PATTERN_LOOP_3+24:
		 case DDR_PATTERN_LOOP_3+25:
		 case DDR_PATTERN_LOOP_3+26:
		 case DDR_PATTERN_LOOP_3+27:
			   	*(p+i) =~0x00004000;
				  break;
               case DDR_PATTERN_LOOP_3+28:
		 case DDR_PATTERN_LOOP_3+29:
		 case DDR_PATTERN_LOOP_3+30:
		 case DDR_PATTERN_LOOP_3+31:
			   	*(p+i) =~0x00008000;	
				  break;
                       
                      
                }
            }

            if(m_len >( 128*4))
            {
                m_len -=( 128*4);
                p += 32*4;
            }
            else
            {
                p += (m_len>>2);
                m_len = 0;
                break;
            }
        }
    }
}

static void ddr_read_pattern4_cross_talk_n(void *buff, unsigned m_length)
{
    unsigned *p;
  //  unsigned i, j, n;
	 unsigned i, n;
    unsigned m_len = m_length;

    p = (unsigned *)buff;

    while(m_len)
    {
      //  for(j=0;j<32;j++)
        {
            if(m_len >= 128*4)
                n = 32*4;
            else
                n = m_len>>2;

            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                     case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 8:
                    case 9:
                    case 10:
                    case 11:
		      case 16:
                    case 17:
                    case 18:
                    case 19:
                    case 24:
                    case 25:
                    case 26:
                    case 27:
                 //   case 30:
                  //      *(p+i) = TDATA32F;
                        if(*(p+i) !=~TDATA32F)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~TDATA32F);
                        break;
                     case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
		      case 20:
                    case 21:
                    case 22:
                    case 23:
		      case 28:
                    case 29:
                    case 30:
                    case 31:
                 //   case 22:
                     //   *(p+i) = 0;
                        if(*(p+i) !=~0)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0);
                        break;
                    case DDR_PATTERN_LOOP_1+0:
                    case DDR_PATTERN_LOOP_1+1:
                    case DDR_PATTERN_LOOP_1+2:
                    case DDR_PATTERN_LOOP_1+3:
                    case DDR_PATTERN_LOOP_1+8:
                    case DDR_PATTERN_LOOP_1+9:
                    case DDR_PATTERN_LOOP_1+10:
                    case DDR_PATTERN_LOOP_1+11:
		      case DDR_PATTERN_LOOP_1+16:
                    case DDR_PATTERN_LOOP_1+17:
                    case DDR_PATTERN_LOOP_1+18:
                    case DDR_PATTERN_LOOP_1+19:
                    case DDR_PATTERN_LOOP_1+24:
                    case DDR_PATTERN_LOOP_1+25:
                    case DDR_PATTERN_LOOP_1+26:
                    case DDR_PATTERN_LOOP_1+27:
                 //   case 30:
                  //        *(p+i) = TDATA32A;
                        if(*(p+i) != ~TDATA32A)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i),~TDATA32A);
                        break;
                       case DDR_PATTERN_LOOP_1+4:
                    case DDR_PATTERN_LOOP_1+5:
                    case DDR_PATTERN_LOOP_1+6:
                    case DDR_PATTERN_LOOP_1+7:
                    case DDR_PATTERN_LOOP_1+12:
                    case DDR_PATTERN_LOOP_1+13:
                    case DDR_PATTERN_LOOP_1+14:
                    case DDR_PATTERN_LOOP_1+15:
		      case DDR_PATTERN_LOOP_1+20:
                    case DDR_PATTERN_LOOP_1+21:
                    case DDR_PATTERN_LOOP_1+22:
                    case DDR_PATTERN_LOOP_1+23:
		      case DDR_PATTERN_LOOP_1+28:
                    case DDR_PATTERN_LOOP_1+29:
                    case DDR_PATTERN_LOOP_1+30:
                    case DDR_PATTERN_LOOP_1+31:
			//	*(p+i) = TDATA325;	
                        if(*(p+i) != ~TDATA325)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~TDATA325);
                        break;
                     case DDR_PATTERN_LOOP_2+0:
		 case DDR_PATTERN_LOOP_2+1:
		 case DDR_PATTERN_LOOP_2+2:
		 case DDR_PATTERN_LOOP_2+3:
			//   	*(p+i) =0xfe01fe01;
                        if(*(p+i) !=~0xfe01fe01)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xfe01fe01);
                        break;
                    case DDR_PATTERN_LOOP_2+4:
		 case DDR_PATTERN_LOOP_2+5:
		 case DDR_PATTERN_LOOP_2+6:
		 case DDR_PATTERN_LOOP_2+7:
			  // 	*(p+i) =0xfd02fd02;
                        if(*(p+i) != ~0xfd02fd02)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xfd02fd02);
                        break;

               case DDR_PATTERN_LOOP_2+8:
		 case DDR_PATTERN_LOOP_2+9:
		 case DDR_PATTERN_LOOP_2+10:
		 case DDR_PATTERN_LOOP_2+11:
			   //	*(p+i) =0xfb04fb04;
			     if(*(p+i) != ~0xfb04fb04)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xfb04fb04);
				  break;
               case DDR_PATTERN_LOOP_2+12:
		 case DDR_PATTERN_LOOP_2+13:
		 case DDR_PATTERN_LOOP_2+14:
		 case DDR_PATTERN_LOOP_2+15:
			   //	*(p+i) =0xf7b08f708;	
				  if(*(p+i) != ~0xf708f708)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xf708f708);
				  break;
               case DDR_PATTERN_LOOP_2+16:
		 case DDR_PATTERN_LOOP_2+17:
		 case DDR_PATTERN_LOOP_2+18:
		 case DDR_PATTERN_LOOP_2+19:
			   //	*(p+i) =0xef10ef10;	
				  if(*(p+i) != ~0xef10ef10)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xef10ef10);
				  break;
               case DDR_PATTERN_LOOP_2+20:
		 case DDR_PATTERN_LOOP_2+21:
		 case DDR_PATTERN_LOOP_2+22:
		 case DDR_PATTERN_LOOP_2+23:
			   //	*(p+i) =0xdf20df20;	
				  if(*(p+i) != ~0xdf20df20)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xdf20df20);
				  break;
               case DDR_PATTERN_LOOP_2+24:
		 case DDR_PATTERN_LOOP_2+25:
		 case DDR_PATTERN_LOOP_2+26:
		 case DDR_PATTERN_LOOP_2+27:
			 //  	*(p+i) =0xbf40bf40;
				  if(*(p+i) != ~0xbf40bf40)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xbf40bf40);
				  break;
               case DDR_PATTERN_LOOP_2+28:
		 case DDR_PATTERN_LOOP_2+29:
		 case DDR_PATTERN_LOOP_2+30:
		 case DDR_PATTERN_LOOP_2+31:
			//   	*(p+i) =0x7f807f80;	
				  if(*(p+i) != ~0x7f807f80)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0x7f807f80);
				  break;
				  break;
               case DDR_PATTERN_LOOP_3+0:
		 case DDR_PATTERN_LOOP_3+1:
		 case DDR_PATTERN_LOOP_3+2:
		 case DDR_PATTERN_LOOP_3+3:
			   //	*(p+i) =0x00000100;
					  if(*(p+i) != ~0x00000100)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0x00000100);
				  break;
               case DDR_PATTERN_LOOP_3+4:
		 case DDR_PATTERN_LOOP_3+5:
		 case DDR_PATTERN_LOOP_3+6:
		 case DDR_PATTERN_LOOP_3+7:
			 //  	*(p+i) =0x00000100;
				  if(*(p+i) != ~0x00000200)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0x00000200);
				  break;
               case DDR_PATTERN_LOOP_3+8:
		 case DDR_PATTERN_LOOP_3+9:
		 case DDR_PATTERN_LOOP_3+10:
		 case DDR_PATTERN_LOOP_3+11:
			   //	*(p+i) =0x00000100;
				 if(*(p+i) != ~0x00000400)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0x00000400);
				  break;
               case DDR_PATTERN_LOOP_3+12:
		 case DDR_PATTERN_LOOP_3+13:
		 case DDR_PATTERN_LOOP_3+14:
		 case DDR_PATTERN_LOOP_3+15:
			   //	*(p+i) =0x00000100;	
				 if(*(p+i) != ~0x00000800)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0x00000800);
				  break;
               case DDR_PATTERN_LOOP_3+16:
		 case DDR_PATTERN_LOOP_3+17:
		 case DDR_PATTERN_LOOP_3+18:
		 case DDR_PATTERN_LOOP_3+19:
			   //	*(p+i) =0xfffffeff;	
				 if(*(p+i) != ~0x00001000)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0x00001000);
				  break;
               case DDR_PATTERN_LOOP_3+20:
		 case DDR_PATTERN_LOOP_3+21:
		 case DDR_PATTERN_LOOP_3+22:
		 case DDR_PATTERN_LOOP_3+23:
			  // 	*(p+i) =0xfffffeff;	
				 if(*(p+i) != ~0x00002000)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0x00002000);
				  break;
               case DDR_PATTERN_LOOP_3+24:
		 case DDR_PATTERN_LOOP_3+25:
		 case DDR_PATTERN_LOOP_3+26:
		 case DDR_PATTERN_LOOP_3+27:
			   //	*(p+i) =0xfffffeff;
				 if(*(p+i) != ~0x00004000)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0x00004000);
				  break;
               case DDR_PATTERN_LOOP_3+28:
		 case DDR_PATTERN_LOOP_3+29:
		 case DDR_PATTERN_LOOP_3+30:
		 case DDR_PATTERN_LOOP_3+31:
			   //	*(p+i) =0xfffffeff;	
				 if(*(p+i) != ~0x00008000)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0x00008000);
				  break;
                       

						
                }
            }

            if(m_len > 128*4)
            {
                m_len -= 128*4;
                p += 32*4;
            }
            else
            {
                p += (m_len>>2);
                m_len = 0;
                break;
            }
        }
    }
}


static void ddr_write_pattern4_no_cross_talk(void *buff, unsigned m_length)
{
    unsigned *p;
 //   unsigned i, j, n;
	 unsigned i, n;
    unsigned m_len = m_length;
//#define ddr_pattern_loop 32
    p = (unsigned *)buff;

    while(m_len)
    {
      //  for(j=0;j<32;j++)
        {
            if(m_len >= 128*4)
                n = 32*4;
            else
                n = m_len>>2;

            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
				 *(p+i) = 0xff00ff00;	
				     break;
		      case 4:
                    case 5:
                    case 6:
                    case 7:
				 *(p+i) = 0xffff0000;	
				     break;
						
                    case 8:
                    case 9:
                    case 10:
                    case 11:
				 *(p+i) = 0xff000000;	
				     break;
		      case 12:
                    case 13:
                    case 14:
                    case 15:
				 *(p+i) = 0xff00ffff;	
				     break;
						
		      case 16:
                    case 17:
                    case 18:
                    case 19:
				 *(p+i) = 0xff00ffff;	
				     break;
		      case 20:
                    case 21:
                    case 22:
                    case 23:	
				  *(p+i) = 0xff0000ff;	
						   break;
                    case 24:
                    case 25:
                    case 26:
                    case 27:
				  *(p+i) = 0xffff0000;	
					   break;
                                  
		      case 28:
                    case 29:
                    case 30:
                    case 31:
                                 *(p+i) = 0x00ff00ff;	
						   break;
               case DDR_PATTERN_LOOP_1+0:
		 case DDR_PATTERN_LOOP_1+1:
		 case DDR_PATTERN_LOOP_1+2:
		 case DDR_PATTERN_LOOP_1+3:
			   	*(p+i) =~0xff00ff00;
				  break;
               case DDR_PATTERN_LOOP_1+4:
		 case DDR_PATTERN_LOOP_1+5:
		 case DDR_PATTERN_LOOP_1+6:
		 case DDR_PATTERN_LOOP_1+7:
			   	*(p+i) =~0xffff0000;
				  break;
               case DDR_PATTERN_LOOP_1+8:
		 case DDR_PATTERN_LOOP_1+9:
		 case DDR_PATTERN_LOOP_1+10:
		 case DDR_PATTERN_LOOP_1+11:
			   	*(p+i) =~0xff000000;
				  break;
               case DDR_PATTERN_LOOP_1+12:
		 case DDR_PATTERN_LOOP_1+13:
		 case DDR_PATTERN_LOOP_1+14:
		 case DDR_PATTERN_LOOP_1+15:
			   	*(p+i) =~0xff00ffff;	
				  break;
               case DDR_PATTERN_LOOP_1+16:
		 case DDR_PATTERN_LOOP_1+17:
		 case DDR_PATTERN_LOOP_1+18:
		 case DDR_PATTERN_LOOP_1+19:
			   	*(p+i) =~0xff00ffff;	
				  break;
               case DDR_PATTERN_LOOP_1+20:
		 case DDR_PATTERN_LOOP_1+21:
		 case DDR_PATTERN_LOOP_1+22:
		 case DDR_PATTERN_LOOP_1+23:
			   	*(p+i) =~0xff00ffff;	
				  break;
               case DDR_PATTERN_LOOP_1+24:
		 case DDR_PATTERN_LOOP_1+25:
		 case DDR_PATTERN_LOOP_1+26:
		 case DDR_PATTERN_LOOP_1+27:
			   	*(p+i) =~0xffff0000;
				  break;
               case DDR_PATTERN_LOOP_1+28:
		 case DDR_PATTERN_LOOP_1+29:
		 case DDR_PATTERN_LOOP_1+30:
		 case DDR_PATTERN_LOOP_1+31:
			   	*(p+i) =~0x00ff00ff;	
				  break;     
		     
               case DDR_PATTERN_LOOP_2+0:
		 case DDR_PATTERN_LOOP_2+1:
		 case DDR_PATTERN_LOOP_2+2:
		 case DDR_PATTERN_LOOP_2+3:
			   	*(p+i) =0x00ff0000;
				  break;
               case DDR_PATTERN_LOOP_2+4:
		 case DDR_PATTERN_LOOP_2+5:
		 case DDR_PATTERN_LOOP_2+6:
		 case DDR_PATTERN_LOOP_2+7:
			   	*(p+i) =0xff000000;
				  break;
               case DDR_PATTERN_LOOP_2+8:
		 case DDR_PATTERN_LOOP_2+9:
		 case DDR_PATTERN_LOOP_2+10:
		 case DDR_PATTERN_LOOP_2+11:
			   	*(p+i) =0x0000ffff;
				  break;
               case DDR_PATTERN_LOOP_2+12:
		 case DDR_PATTERN_LOOP_2+13:
		 case DDR_PATTERN_LOOP_2+14:
		 case DDR_PATTERN_LOOP_2+15:
			   	*(p+i) =0x000000ff;	
				  break;
               case DDR_PATTERN_LOOP_2+16:
		 case DDR_PATTERN_LOOP_2+17:
		 case DDR_PATTERN_LOOP_2+18:
		 case DDR_PATTERN_LOOP_2+19:
			   	*(p+i) =0x00ff00ff;	
				  break;
               case DDR_PATTERN_LOOP_2+20:
		 case DDR_PATTERN_LOOP_2+21:
		 case DDR_PATTERN_LOOP_2+22:
		 case DDR_PATTERN_LOOP_2+23:
			   	*(p+i) =0xff00ff00;	
				  break;
               case DDR_PATTERN_LOOP_2+24:
		 case DDR_PATTERN_LOOP_2+25:
		 case DDR_PATTERN_LOOP_2+26:
		 case DDR_PATTERN_LOOP_2+27:
			   	*(p+i) =0xff00ffff;
				  break;
               case DDR_PATTERN_LOOP_2+28:
		 case DDR_PATTERN_LOOP_2+29:
		 case DDR_PATTERN_LOOP_2+30:
		 case DDR_PATTERN_LOOP_2+31:
			   	*(p+i) =0xff00ff00;	
				  break;
               case DDR_PATTERN_LOOP_3+0:
		 case DDR_PATTERN_LOOP_3+1:
		 case DDR_PATTERN_LOOP_3+2:
		 case DDR_PATTERN_LOOP_3+3:
			   	*(p+i) =~0x00ff0000;
				  break;
               case DDR_PATTERN_LOOP_3+4:
		 case DDR_PATTERN_LOOP_3+5:
		 case DDR_PATTERN_LOOP_3+6:
		 case DDR_PATTERN_LOOP_3+7:
			   	*(p+i) =~0xff000000;
				  break;
               case DDR_PATTERN_LOOP_3+8:
		 case DDR_PATTERN_LOOP_3+9:
		 case DDR_PATTERN_LOOP_3+10:
		 case DDR_PATTERN_LOOP_3+11:
			   	*(p+i) =~0x0000ffff;
				  break;
               case DDR_PATTERN_LOOP_3+12:
		 case DDR_PATTERN_LOOP_3+13:
		 case DDR_PATTERN_LOOP_3+14:
		 case DDR_PATTERN_LOOP_3+15:
			   	*(p+i) =~0x000000ff;	
				  break;
               case DDR_PATTERN_LOOP_3+16:
		 case DDR_PATTERN_LOOP_3+17:
		 case DDR_PATTERN_LOOP_3+18:
		 case DDR_PATTERN_LOOP_3+19:
			   	*(p+i) =~0x00ff00ff;	
				  break;
               case DDR_PATTERN_LOOP_3+20:
		 case DDR_PATTERN_LOOP_3+21:
		 case DDR_PATTERN_LOOP_3+22:
		 case DDR_PATTERN_LOOP_3+23:
			   	*(p+i) =~0xff00ff00;	
				  break;
               case DDR_PATTERN_LOOP_3+24:
		 case DDR_PATTERN_LOOP_3+25:
		 case DDR_PATTERN_LOOP_3+26:
		 case DDR_PATTERN_LOOP_3+27:
			   	*(p+i) =~0xff00ffff;
				  break;
               case DDR_PATTERN_LOOP_3+28:
		 case DDR_PATTERN_LOOP_3+29:
		 case DDR_PATTERN_LOOP_3+30:
		 case DDR_PATTERN_LOOP_3+31:
			   	*(p+i) =~0xff00ff00;	
				  break;
                       
                      
                }
            }

            if(m_len >( 128*4))
            {
                m_len -=( 128*4);
                p += 32*4;
            }
            else
            {
                p += (m_len>>2);
                m_len = 0;
                break;
            }
        }
    }
}

static void ddr_read_pattern4_no_cross_talk(void *buff, unsigned m_length)
{
    unsigned *p;
  //  unsigned i, j, n;
	 unsigned i, n;
    unsigned m_len = m_length;

    p = (unsigned *)buff;
  while(m_len)
	{
      //  for(j=0;j<32;j++)
        {
            if(m_len >= 128*4)
                n = 32*4;
            else
                n = m_len>>2;

            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
					//	  if(*(p+i) !=~TDATA32F)
                            
				if( *(p+i) != 0xff00ff00)	
					printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xff00ff00);
                				     break;
		      case 4:
                    case 5:
                    case 6:
                    case 7:
						if( *(p+i) != 0xffff0000)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xffff0000);
                			
				     break;
						
                    case 8:
                    case 9:
                    case 10:
                    case 11:
				// *(p+i) = 0xff000000;	
				 if( *(p+i) != 0xff000000)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xff000000);
                			
				     break;
		      case 12:
                    case 13:
                    case 14:
                    case 15:
				// *(p+i) = 0xff00ffff;	
				 if( *(p+i) != 0xff00ffff)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xff00ffff);
                			
				     break;
						
		      case 16:
                    case 17:
                    case 18:
                    case 19:
			//	 *(p+i) = 0xff00ffff;	
				 if( *(p+i) != 0xff00ffff)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xff00ffff);
                			
				     break;
		      case 20:
                    case 21:
                    case 22:
                    case 23:	
				//  *(p+i) = 0xff0000ff;	
				  if( *(p+i) != 0xff0000ff)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xff0000ff);
                			
						   break;
                    case 24:
                    case 25:
                    case 26:
                    case 27:
				//  *(p+i) = 0xffff0000;	
				  if( *(p+i) != 0xffff0000)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xffff0000);
                			
					   break;
                                  
		      case 28:
                    case 29:
                    case 30:
                    case 31:
                               //  *(p+i) = 0x00ff00ff;	
								 if( *(p+i) != 0x00ff00ff)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0x00ff00ff);
                			
						   break;
               case DDR_PATTERN_LOOP_1+0:
		 case DDR_PATTERN_LOOP_1+1:
		 case DDR_PATTERN_LOOP_1+2:
		 case DDR_PATTERN_LOOP_1+3:
			 //  	*(p+i) =~0xff00ff00;
				if( *(p+i) != ~0xff00ff00)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xff00ff00);
                			
				  break;
               case DDR_PATTERN_LOOP_1+4:
		 case DDR_PATTERN_LOOP_1+5:
		 case DDR_PATTERN_LOOP_1+6:
		 case DDR_PATTERN_LOOP_1+7:
			  // 	*(p+i) =~0xffff0000;
				if( *(p+i) != ~0xffff0000)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xffff0000);
                			
				  break;
               case DDR_PATTERN_LOOP_1+8:
		 case DDR_PATTERN_LOOP_1+9:
		 case DDR_PATTERN_LOOP_1+10:
		 case DDR_PATTERN_LOOP_1+11:
			  // 	*(p+i) =~0xff000000;
				if( *(p+i) != ~0xff000000)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xff000000);
                			
				  break;
               case DDR_PATTERN_LOOP_1+12:
		 case DDR_PATTERN_LOOP_1+13:
		 case DDR_PATTERN_LOOP_1+14:
		 case DDR_PATTERN_LOOP_1+15:
			   //	*(p+i) =~0xff00ffff;	
				if( *(p+i) != ~0xff00ffff)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xff00ffff);
                			
				  break;
               case DDR_PATTERN_LOOP_1+16:
		 case DDR_PATTERN_LOOP_1+17:
		 case DDR_PATTERN_LOOP_1+18:
		 case DDR_PATTERN_LOOP_1+19:
			   //	*(p+i) =~0xff00ffff;	
				if( *(p+i) != ~0xff00ffff)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xff00ffff);
                			
				  break;
               case DDR_PATTERN_LOOP_1+20:
		 case DDR_PATTERN_LOOP_1+21:
		 case DDR_PATTERN_LOOP_1+22:
		 case DDR_PATTERN_LOOP_1+23:
			   //	*(p+i) =~0xff00ffff;	
				if( *(p+i) != ~0xff00ffff)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xff00ffff);
                			
				  break;
               case DDR_PATTERN_LOOP_1+24:
		 case DDR_PATTERN_LOOP_1+25:
		 case DDR_PATTERN_LOOP_1+26:
		 case DDR_PATTERN_LOOP_1+27:
			   //	*(p+i) =~0xffff0000;
				if( *(p+i) != ~0xffff0000)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xffff0000);
                			
				  break;
               case DDR_PATTERN_LOOP_1+28:
		 case DDR_PATTERN_LOOP_1+29:
		 case DDR_PATTERN_LOOP_1+30:
		 case DDR_PATTERN_LOOP_1+31:
			 //  	*(p+i) =~0x00ff00ff;	
				if( *(p+i) != ~0x00ff00ff)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0x00ff00ff);
                			
				  break;     
		     
               case DDR_PATTERN_LOOP_2+0:
		 case DDR_PATTERN_LOOP_2+1:
		 case DDR_PATTERN_LOOP_2+2:
		 case DDR_PATTERN_LOOP_2+3:
			 //  	*(p+i) =0x00ff0000;
				if( *(p+i) != 0x00ff0000)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0x00ff0000);
                			
				  break;
               case DDR_PATTERN_LOOP_2+4:
		 case DDR_PATTERN_LOOP_2+5:
		 case DDR_PATTERN_LOOP_2+6:
		 case DDR_PATTERN_LOOP_2+7:
			//   	*(p+i) =0xff000000;
				if( *(p+i) != 0xff000000)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xff000000);
                			
				  break;
               case DDR_PATTERN_LOOP_2+8:
		 case DDR_PATTERN_LOOP_2+9:
		 case DDR_PATTERN_LOOP_2+10:
		 case DDR_PATTERN_LOOP_2+11:
			  // 	*(p+i) =0x0000ffff;
				if( *(p+i) != 0x0000ffff)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0x0000ffff);
                			
				  break;
               case DDR_PATTERN_LOOP_2+12:
		 case DDR_PATTERN_LOOP_2+13:
		 case DDR_PATTERN_LOOP_2+14:
		 case DDR_PATTERN_LOOP_2+15:
			//   	*(p+i) =0x000000ff;	
				if( *(p+i) != 0x000000ff)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0x000000ff);
                			
				  break;
               case DDR_PATTERN_LOOP_2+16:
		 case DDR_PATTERN_LOOP_2+17:
		 case DDR_PATTERN_LOOP_2+18:
		 case DDR_PATTERN_LOOP_2+19:
			//   	*(p+i) =0x00ff00ff;	
				if( *(p+i) != 0x00ff00ff)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0x00ff00ff);
                			
				  break;
               case DDR_PATTERN_LOOP_2+20:
		 case DDR_PATTERN_LOOP_2+21:
		 case DDR_PATTERN_LOOP_2+22:
		 case DDR_PATTERN_LOOP_2+23:
			   //	*(p+i) =0xff00ff00;	
				if( *(p+i) != 0xff00ff00)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xff00ff00);
                			
				  break;
               case DDR_PATTERN_LOOP_2+24:
		 case DDR_PATTERN_LOOP_2+25:
		 case DDR_PATTERN_LOOP_2+26:
		 case DDR_PATTERN_LOOP_2+27:
			//   	*(p+i) =0xff00ffff;
				if( *(p+i) != 0xff00ffff)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xff00ffff);
                			
				  break;
               case DDR_PATTERN_LOOP_2+28:
		 case DDR_PATTERN_LOOP_2+29:
		 case DDR_PATTERN_LOOP_2+30:
		 case DDR_PATTERN_LOOP_2+31:
		//	   	*(p+i) =0xff00ff00;	
				if( *(p+i) != 0xff00ff00)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0xff00ff00);
                			
				  break;
               case DDR_PATTERN_LOOP_3+0:
		 case DDR_PATTERN_LOOP_3+1:
		 case DDR_PATTERN_LOOP_3+2:
		 case DDR_PATTERN_LOOP_3+3:
			   //	*(p+i) =~0x00ff0000;
				if( *(p+i) != ~0x00ff0000)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0x00ff0000);
                			
				  break;
               case DDR_PATTERN_LOOP_3+4:
		 case DDR_PATTERN_LOOP_3+5:
		 case DDR_PATTERN_LOOP_3+6:
		 case DDR_PATTERN_LOOP_3+7:
			//   	*(p+i) =~0xff000000;
				if( *(p+i) != ~0xff000000)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xff000000);
                			
				  break;
               case DDR_PATTERN_LOOP_3+8:
		 case DDR_PATTERN_LOOP_3+9:
		 case DDR_PATTERN_LOOP_3+10:
		 case DDR_PATTERN_LOOP_3+11:
			 //  	*(p+i) =~0x0000ffff;
				if( *(p+i) != ~0x0000ffff)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0x0000ffff);
                			
				  break;
               case DDR_PATTERN_LOOP_3+12:
		 case DDR_PATTERN_LOOP_3+13:
		 case DDR_PATTERN_LOOP_3+14:
		 case DDR_PATTERN_LOOP_3+15:
			//   	*(p+i) =~0x000000ff;	
				if( *(p+i) != ~0x000000ff)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0x000000ff);
                		
				  break;
               case DDR_PATTERN_LOOP_3+16:
		 case DDR_PATTERN_LOOP_3+17:
		 case DDR_PATTERN_LOOP_3+18:
		 case DDR_PATTERN_LOOP_3+19:
			//   	*(p+i) =~0x00ff00ff;	
				if( *(p+i) != ~0x00ff00ff)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0x00ff00ff);
                			
				  break;
               case DDR_PATTERN_LOOP_3+20:
		 case DDR_PATTERN_LOOP_3+21:
		 case DDR_PATTERN_LOOP_3+22:
		 case DDR_PATTERN_LOOP_3+23:
			   //	*(p+i) =~0xff00ff00;	
				if( *(p+i) != ~0xff00ff00)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xff00ff00);
                			
				  break;
               case DDR_PATTERN_LOOP_3+24:
		 case DDR_PATTERN_LOOP_3+25:
		 case DDR_PATTERN_LOOP_3+26:
		 case DDR_PATTERN_LOOP_3+27:
			 //  	*(p+i) =~0xff00ffff;
				if( *(p+i) != ~0xff00ffff)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xff00ffff);
                			
				  break;
               case DDR_PATTERN_LOOP_3+28:
		 case DDR_PATTERN_LOOP_3+29:
		 case DDR_PATTERN_LOOP_3+30:
		 case DDR_PATTERN_LOOP_3+31:
			//   	*(p+i) =~0xff00ff00;	
				if( *(p+i) != ~0xff00ff00)
							printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~0xff00ff00);
                			
				  break;
                       
                      
                }
            }

            if(m_len >( 128*4))
            {
                m_len -=( 128*4);
                p += 32*4;
            }
            else
            {
                p += (m_len>>2);
                m_len = 0;
                break;
            }
        }
    }
}

#define DDR_TEST_START_ADDR CONFIG_SYS_MEMTEST_START
#define DDR_TEST_SIZE 0x2000000

int do_ddr_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char *endp;
    unsigned long loop = 1;
    unsigned char lflag = 0;
    unsigned start_addr = DDR_TEST_START_ADDR;
     unsigned char simple_pattern_flag = 1;
	    unsigned char cross_talk_pattern_flag = 1;
		 unsigned char old_pattern_flag = 1;
    if(!argc)
        goto DDR_TEST_START;

    if (strcmp(argv[1], "l") == 0){
        lflag = 1;
    }
    else if (strcmp(argv[1], "h") == 0){
        goto usage;
    }
    else{
        loop = simple_strtoul(argv[1], &endp, 10);
        if (*argv[1] == 0 || *endp != 0)
            loop = 1;
    }
    
    if(argc > 2){
        start_addr = simple_strtoul(argv[2], &endp, 16);
        if (*argv[2] == 0 || *endp != 0)
            start_addr = DDR_TEST_START_ADDR;
		
    }
	old_pattern_flag = 1;
	  simple_pattern_flag = 1;
	    cross_talk_pattern_flag = 1;
    if( (strcmp(argv[1], "s") == 0)||(strcmp(argv[2], "s") == 0)||(strcmp(argv[3], "s") == 0))
		{
        simple_pattern_flag = 1;
		old_pattern_flag=0;
		 cross_talk_pattern_flag = 0;
    }
  if ((strcmp(argv[1], "c") == 0)||(strcmp(argv[2], "c") == 0)||(strcmp(argv[3], "c") == 0))
		{
       simple_pattern_flag = 0;
		old_pattern_flag=0;
		 cross_talk_pattern_flag = 1;
    }
	
DDR_TEST_START:

    do{
        if(lflag)
            loop = 888;

		if(old_pattern_flag==1)
			{
        printf("\rStart writing at 0x%08x - 0x%08x...", start_addr, start_addr + DDR_TEST_SIZE);
        ddr_write((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd write.                                 ");
	    printf("\rStart 1st reading...                       ");
	    ddr_read((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd 1st read.                              ");
	    printf("\rStart 2nd reading...                       ");
	    ddr_read((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd 2nd read.                              ");
	    printf("\rStart 3rd reading...                       ");
	    ddr_read((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd 3rd read.                              \n");
			}

///*
if(simple_pattern_flag==1)
{
		    printf("\nStart *4 no cross talk pattern.                                 ");
		  printf("\nStart writing at 0x%08x - 0x%08x...", start_addr, start_addr + DDR_TEST_SIZE);
        ddr_write_pattern4_no_cross_talk((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd write.                                 ");
	    printf("\rStart 1st reading...                       ");
	    ddr_read_pattern4_no_cross_talk((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd 1st read.                              ");
	    printf("\rStart 2nd reading...                       ");
	    ddr_read_pattern4_no_cross_talk((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd 2nd read.                              ");
	    printf("\rStart 3rd reading...                       ");
	    ddr_read_pattern4_no_cross_talk((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd 3rd read.                              \n");
}

if(cross_talk_pattern_flag==1)
{
		    printf("\nStart *4  cross talk pattern p.                                 ");
		  printf("\nStart writing at 0x%08x - 0x%08x...", start_addr, start_addr + DDR_TEST_SIZE);
        ddr_write_pattern4_cross_talk_p((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd write.                                 ");
	    printf("\rStart 1st reading...                       ");
	    ddr_read_pattern4_cross_talk_p((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd 1st read.                              ");
	    printf("\rStart 2nd reading...                       ");
	    ddr_read_pattern4_cross_talk_p((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd 2nd read.                              ");
	    printf("\rStart 3rd reading...                       ");
	    ddr_read_pattern4_cross_talk_p((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd 3rd read.                              \n");

			    printf("\nStart *4  cross talk pattern n.                                 ");
		  printf("\nStart writing at 0x%08x - 0x%08x...", start_addr, start_addr + DDR_TEST_SIZE);
        ddr_write_pattern4_cross_talk_n((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd write.                                 ");
	    printf("\rStart 1st reading...                       ");
	    ddr_read_pattern4_cross_talk_n((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd 1st read.                              ");
	    printf("\rStart 2nd reading...                       ");
	    ddr_read_pattern4_cross_talk_n((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd 2nd read.                              ");
	    printf("\rStart 3rd reading...                       ");
	    ddr_read_pattern4_cross_talk_n((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd 3rd read.                              \n");

	
}
		//*/
	  }while(--loop);

	  return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	ddrtest,	5,	1,	do_ddr_test,
	"DDR test function",
	"ddrtest [LOOP] [ADDR].Default address is 0x8d000000\n"
);
