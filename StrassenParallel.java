import java.util.*;
import java.math.*;
class StrassenParallel
{
   
	public static void main(String[] args)
	{
		int n = 8;
		int i = 0;
                int j = 0; 
                int k = 0;
		for(k=0;k<9;k++)
	{
		n = n*2;
		System.out.println(n);
		int min = 1;
		int max = 50;
		float [][]a = new float[n][n];
		float [][]b = new float[n][n];
		float [][]c = new float[n][n];
		for(i=0;i<n;i++)
		{
			for(j=0;j<n;j++)
			{
				a[i][j] = (float)(min + (int)(Math.random() * max));
				b[i][j] = (float)(min + (int)(Math.random() * max));
			}
		}	
		double start = System.nanoTime();
		c = strassen(a,b);
		double end = System.nanoTime();

		System.out.println((end-start)/1000000000 + " Seconds");
		
	}			
	}

public static float[][] strassen(float[][]a, float[][]b)
	{	
		int n = a.length;
		float[][] R = new float[n][n];
		if(n==1)
			R[0][0] = a[0][0]*b[0][0];
		

		else//generally parallel better for lower sizes, should put if for general use
		{
			float[][] a11 = new float[n/2][n/2];
			float[][] a12 = new float[n/2][n/2];
			float[][] a21 = new float[n/2][n/2];
			float[][] a22 = new float[n/2][n/2];
			float[][] b11 = new float[n/2][n/2];
			float[][] b12 = new float[n/2][n/2];
			float[][] b21 = new float[n/2][n/2];
			float[][] b22 = new float[n/2][n/2];	

			class Mul implements Runnable
			{
				private Thread t;
				private String threadName;
				Mul(String name)
				{
					threadName=name;
				}
				public void run()
				{
					if(threadName.equals("thread1"))
					{
						split(a,a11,0,0);
					}
					else if(threadName.equals("thread2"))
					{
						 split(a,a12,0,n/2);			
					}
		
					else if(threadName.equals("thread3"))
					{
						  split(a,a21,n/2,0);
					}
				
					else if(threadName.equals("thread4"))
					{
						split(a,a22,n/2,n/2);
					}

					else if(threadName.equals("thread5"))
					{
						split(b,b11,0,0);
					}
			
					else if(threadName.equals("thread6"))
					{
						 split(b,b12,0,n/2);
					}
		
					else if(threadName.equals("thread7"))
					{
						split(b,b21,n/2,0);
					}

					else if(threadName.equals("thread8"))
					{
						split(b,b22,n/2,n/2);
					}
				}
			
				public void start()
				{
					if(t==null)
					{
						t=new Thread(this,"Thread1");
						t.start();
					}
				}
			}
			Mul m1=new Mul("thread1");
			m1.start();
			Mul m2=new Mul("thread2");
			m2.start();
			Mul m3=new Mul("thread3");
			m3.start();
			Mul m4=new Mul("thread4");
			m4.start();
			Mul m5=new Mul("thread5");
			m5.start();
			Mul m6=new Mul("thread6");
			m6.start();
			Mul m7=new Mul("thread7");
			m7.start();
			Mul m8=new Mul("thread8");
			m8.start();
			for (Thread t : new Thread[] { m1.t,m2.t,m3.t,m4.t,m5.t,m6.t,m7.t,m8.t })
			{
				try
				{
					t.join();
				}
				catch(Exception e)
				{
					System.out.println(e);
				}
			}

			
			float [][] M1 = strassen(add(a11, a22), add(b11, b22));
           		float [][] M2 = strassen(add(a21, a22), b11);
            		float [][] M3 = strassen(a11, sub(b12, b22));
            		float [][] M4 = strassen(a22, sub(b21, b11));
            		float [][] M5 = strassen(add(a11, a12), b22);
            		float [][] M6 = strassen(sub(a21, a11), add(b11, b12));
            		float [][] M7 = strassen(sub(a12, a22), add(b21, b22));

			float [][] C11 = add(sub(add(M1, M4), M5), M7);
            		float [][] C12 = add(M3, M5);
            		float [][] C21 = add(M2, M4);
            		float [][] C22 = add(sub(add(M1, M3), M2), M6);

			class Mul2 implements Runnable
			{
				private Thread t;
				private String threadName;
				Mul2(String name)
				{
					threadName=name;
				}

				public void run()
				{
					if(threadName.equals("thread1"))
					{
						join(C11, R, 0 , 0);
					}
					else if(threadName.equals("thread2"))
					{
						 join(C12, R, 0 , n/2);			
					}
		
					else if(threadName.equals("thread3"))
					{
						 join(C21, R, n/2, 0);
            		
					}
				
					else if(threadName.equals("thread4"))
					{
						 join(C22, R, n/2, n/2);
					}

				}

				public void start()
				{
					if(t==null)
					{
						t=new Thread(this,"Thread1");
						t.start();
					}
				}
			}
			Mul2 m11=new Mul2("thread1");
			m11.start();
			Mul2 m22=new Mul2("thread2");
			m22.start();
			Mul2 m33=new Mul2("thread3");
			m33.start();
			Mul2 m44=new Mul2("thread4");
			m44.start();
			for (Thread t : new Thread[] { m11.t,m22.t,m33.t,m44.t })
			{
				try
				{
					t.join();
				}
				catch(Exception e)
				{
					System.out.println(e);
				}
			}
		}
		return R;
	}
	
public static void split(float[][]P,float[][]C,int iB,int jB)
	{
		int i2 = iB;
		for(int i1=0;i1< C.length;i1++)
		{
			int j2 = jB;
			for(int j1=0; j1<C.length;j1++)
			{
				C[i1][j1] = P[i2][j2];
				j2++;
			}
			i2++;
		}
	}

public static float[][] add(float[][] a,float[][] b)
	{
		int n = a.length;
		float[][] c = new float[n][n];
		for(int i = 0;i<n;i++)
		{
			for(int j=0;j<n;j++)
				c[i][j] = a[i][j] + b[i][j];
		}
	return c;
	}

public static float[][] sub(float[][] a,float[][] b)
	{
		int n = a.length;
		float[][] c = new float[n][n];
		for(int i = 0;i<n;i++)
		{
			for(int j=0;j<n;j++)
				c[i][j] = a[i][j] - b[i][j];
		}
	return c;
	}
public static void join(float[][]P,float[][]C,int iB,int jB)
	{
		int i2 = iB;
		for(int i1=0;i1< P.length;i1++)
		{
			int j2 = jB;
			for(int j1=0; j1< P.length;j1++)
			{
				C[i2][j2] = P[i1][j1];
				j2++;
			}
			i2++;
		}
	}
}
