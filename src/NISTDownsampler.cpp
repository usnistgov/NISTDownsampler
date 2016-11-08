/*******************************************************************************

License: 
This software was developed at the National Institute of Standards and 
Technology (NIST) by employees of the Federal Government in the course 
of their official duties. Pursuant to title 17 Section 105 of the 
United States Code, this software is not subject to copyright protection 
and is in the public domain. NIST assumes no responsibility  whatsoever for 
its use by other parties, and makes no guarantees, expressed or implied, 
about its quality, reliability, or any other characteristic. 

This software has been determined to be outside the scope of the EAR
(see Part 734.3 of the EAR for exact details) as it has been created solely
by employees of the U.S. Government; it is freely distributed with no
licensing requirements; and it is considered public domain.  Therefore,
it is permissible to distribute this software as a free download from the
internet.

Disclaimer: 
This software was developed to promote biometric standards and biometric
technology testing for the Federal Government in accordance with the USA
PATRIOT Act and the Enhanced Border Security and Visa Entry Reform Act.
Specific hardware and software products identified in this software were used
in order to perform the software development.  In no case does such
identification imply recommendation or endorsement by the National Institute
of Standards and Technology, nor does it imply that the products and equipment
identified are necessarily the best available for the purpose.  

*******************************************************************************/

/*******************************************************************************
	PACKAGE:	NISTDownsampler

	FILE:		NISTDOWNSAMPLER.CPP

	AUTHORS:	John D. Grantham
	
	DATE:		12/17/2012
	
	UPDATED:	09/28/2016
				10/28/2016

	
	DESCRIPTION:

		An application which will downsample a given image using a Guassian filter
		followed by decimation, based on the method recommended in NIST IR 7839
		and NIST SP 500-289. 

*******************************************************************************/

/* Win32 includes */
#ifdef WIN32
/* Intentionally blank -- a placeholder for any Win32-specific includes */

/* Linux includes */
#else 
/* #include <gtk/gtk.h> */

#endif

/* C++ Includes */
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

using namespace std;

/* OpenCV includes */
#include "opencv/cv.h"
#include "opencv/highgui.h"

using namespace cv;

/* NIST DOWNSAMPLER ID (version 1.0.0) */
#define DOWNSAMPLER_ID "DsmID: NIST-000000000000100 Resvd: cf3357659812d6ba14d52225977cfdcf6e863d20e04567744c1bfd1e7c9acb27 " 


/*******************************************************************************
FUNCTION NAME:	print_usage()

AUTHOR:			John D. Grantham

DESCRIPTION:	Prints the usage instructions to the screen (stderr). 

	INPUT:
		executable		- A string containing the executable name

	OUTPUT:
		VOID			- Usage instructions are printed to stderr

*******************************************************************************/
void print_usage (const string executable)
{
	cerr << "\nUSAGE: " << executable << " [INPUT IMAGE] [OUTPUT IMAGE]\n(Output image must be .PGM)\n";
}


/*******************************************************************************
FUNCTION NAME:	roundbanker()

AUTHOR:			John D. Grantham

DESCRIPTION:	Rounds a double-precision value to an int, using the Banker's
				rounding method (round to nearest even number). 

	INPUT:
		val				- A double-precision value

	OUTPUT:
		return			- The integer result of the rounding operation

*******************************************************************************/
int roundbanker (double val)
{
	if ((val - (int)val) < 0.5)
	{
		return floor(val);
	}
	else if ((val - (int)val) > 0.5)
	{
		return ceil(val);
	}
	else
	{
		/* If value is split between two ints, round to nerest EVEN integer */
		if (((int)val % 2) == 0) // is even
			return floor(val);
		else
			return ceil(val);
	}
}


/*******************************************************************************
FUNCTION NAME:	point_distance()

AUTHOR:			John D. Grantham

DESCRIPTION:	Calculates and returns the distance between two points


	INPUT:
		a				- A CvPoint (origin)
		b				- Another CvPoint (destination)

	OUTPUT:
		return			- The distance between the two given CvPoints (from
						  origin to destination)

*******************************************************************************/
double point_distance(Point a, Point b)
{
	return sqrt((double)((b.x - a.x)*(b.x - a.x)) + ((b.y - a.y)*(b.y - a.y)));
}

/*******************************************************************************
FUNCTION NAME:	gaussian()

AUTHOR:			John D. Grantham

DESCRIPTION:	Returns the value of the Gaussian function for a given input
				value, mu, and sigma. 

	INPUT:
		x				- A double-precision input value
		mu				- An int for the mu variable in the Gaussian function
		sigma			- A double for the sigma value in the Gaussian function

	OUTPUT:
		return			- The double-precision result of the Gaussian function

*******************************************************************************/
double gaussian(double x, int mu, double sigma)
{
	return (1 / sqrt(2 * CV_PI * sigma * sigma)) * exp(-((x - mu) * (x - mu)) / (2 * sigma * sigma));
}


/*******************************************************************************
FUNCTION NAME:	GaussianFilter()

AUTHOR:			John D. Grantham

DESCRIPTION:	Constructs a two-dimensional (square) Gaussian kernel with the 
				given radius and sigma values, applies the kernel to the given 
				source image, and writes the result to a given destination image. 

	INPUT:
		src				- A pointer to a source image of type IplImage
		dst				- A pointer to a destination image of type IplImage
		sigma			- A double for the sigma value in the Gaussian function
		radius			- An int for the radius of the Gaussian kernel

	OUTPUT:
		void			- (The filtered result is written to "dst" above)

*******************************************************************************/
void GaussianFilter (IplImage* src, IplImage* dst, double sigma, int radius)
{
	/* Construct kernel */
	int length = radius * 2 + 1;
	CvMat* gaussian_kernel = cvCreateMat(length, length, CV_32F);
    Point center = Point(radius, radius);
	
	/* Calculate distances */
	for (int x = 0; x < length; x++)
	{
		for (int y = 0; y < length; y++)
		{
			((float *)(gaussian_kernel->data.ptr + gaussian_kernel->step * y))[x] = point_distance(Point(x,y), center);
		}
	}

	/* Calculate values based on distances */
	double sum = 0;
    for (int x = 0; x < length; x++)
	{
		for (int y = 0; y < length; y++)
        {
			((float *)(gaussian_kernel->data.ptr + gaussian_kernel->step * y))[x] = gaussian(((float *)(gaussian_kernel->data.ptr + gaussian_kernel->step * y))[x], 0, sigma);
			sum+= ((float *)(gaussian_kernel->data.ptr + gaussian_kernel->step * y))[x];
        }
	}
	for (int x = 0; x < length; x++)
	{
		for (int y = 0; y < length; y++)
		{
			((float *)(gaussian_kernel->data.ptr + gaussian_kernel->step * y))[x] /= sum;
		}
	}

	/* Apply filter */
	CvMat* area = cvCreateMat(length, length, CV_8U);
	for (int y = 0; y < src->height; y++)
	{
		for (int x = 0; x < src->width; x++)
        {
			/* get surrounding pixels */
			for (int y2 = 0; y2 < length; y2++)
			{
				for (int x2 = 0; x2 < length; x2++)
                {
					int xbound = x - radius + x2;
					int ybound = y - radius + y2;
					if (xbound >= 0 && xbound < src->width && ybound >= 0 && ybound < src->height)
					{
						((uchar *)(area->data.ptr + area->step * y2))[x2] = ((uchar *)(src->imageData + src->widthStep * (y - radius + y2)))[(x - radius + x2)];
					}
                    else // if filter goes off the edge, use nearest pixel
					{
						((uchar *)(area->data.ptr + area->step * y2))[x2] = ((uchar *)(src->imageData + src->widthStep * y))[x];
					}
				}
			}
	
            /* find weighted sum */
            double wsum = 0;
            for (int x2 = 0; x2 < length; x2++)
			{
                for (int y2 = 0; y2 < length; y2++)
				{
					wsum += (double)((uchar *)(area->data.ptr + area->step * y2))[x2] * ((float *)(gaussian_kernel->data.ptr + gaussian_kernel->step * y2))[x2];
				}
			}
			/* Round weighted sum to an integer using banker's rounding (instead of truncation) */
			((uchar *)(dst->imageData + dst->widthStep * y))[x] = roundbanker(wsum);
		}
	}
}


/*******************************************************************************
FUNCTION NAME:	DecimateImage()

AUTHOR:			John D. Grantham

DESCRIPTION:	Decimates a given source image, removing every odd row and 
				column, and stores the result in the given destination image.

	INPUT:
		src				- A pointer to a source image of type IplImage
		dst				- A pointer to a destination image of type IplImage
		
	OUTPUT:
		void			- (The filtered result is written to "dst" above)

*******************************************************************************/
void DecimateImage(IplImage* src, IplImage* dst)
{
	/* Save odd rows and columns, per official recommendation */
	for (int y = 0; y < src->height; y++)
	{
		if (y % 2 == 1)
		{
			for (int x = 0; x < src->width; x++)
			{
				if (x % 2 == 1)
				{
					((uchar *)(dst->imageData + dst->widthStep * (y / 2)))[(x / 2)] = ((uchar *)(src->imageData + src->widthStep * y))[x];
				}
			}
		}
	}
}


/*******************************************************************************
FUNCTION NAME:	CommentPGM()

AUTHOR:			John D. Grantham

DESCRIPTION:	Adds a comment to a PGM image file

INPUT:
	pgmfile		- A string containing the path to the pgm file
	comment		- A string containing the comment to be written

OUTPUT:
	void			

*******************************************************************************/
void CommentPGM(const string pgmfile, const string comment)
{
	ifstream infile;

	infile.open(pgmfile, ios::binary | ios::ate);
	if (infile.fail())
	{
		throw iostream::failure(pgmfile);
	}
	else
	{
		/* Read file into buffer */
		streamsize size = infile.tellg();
		infile.seekg(0, ios::beg);

		vector<char> buffer(size);
		infile.read(buffer.data(), size);
		infile.close();

		/* Write buffer out to file, insert comment */
		ofstream outfile;
		outfile.open(pgmfile, ios::binary | ios::ate);

		if (outfile.fail())
		{
			throw iostream::failure(pgmfile);
		}
		else
		{
			/* Insert comment after first three characters */
			outfile << buffer.at(0) << buffer.at(1) << buffer.at(2);
			outfile << "#" << comment << endl;

			/* Write remaining buffer */
			for (int i = 3; i < size; i++)
			{
				outfile << buffer.at(i);
			}
			outfile.close();
		}
	}
}

/*******************************************************************************
FUNCTION NAME:	main()

AUTHOR:			John D. Grantham

DESCRIPTION:	Accepts command-line arguments and runs the application.

	INPUT:
		argc			- The number of command line arguments
		argv			- An array of command line arguments

	OUTPUT:
		return			- Return code indicating execution success or failure

*******************************************************************************/
int main (int argc, char **argv)
{
	try
	{
		/* Constants and variables */
		const int radius = 4;
		const double sigma = 0.8475;
		ofstream outfile;
		string outfilename;

		/* Parse command-line arguments */
		if (argc != 3)
		{
			cerr << "ERROR: Incorrect number of arguments!\n";
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}
		else // Check command line args
		{
			if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "-help") || !strcmp(argv[1], "?"))
			{
				print_usage(argv[0]);
				exit(EXIT_SUCCESS);
			}
			else
			{
				/* Test output file */
				outfilename = argv[2];
				string ext = outfilename.substr(outfilename.find_last_of(".") + 1);
				std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

				if (ext == "pgm")
				{
					outfile.open(argv[2]);
					if (outfile.fail())
					{
						cerr << "ERROR: Cannot open output file: \"" << argv[2] << "\"!\n";
						print_usage(argv[0]);
						exit(EXIT_FAILURE);
					}
					else
					{
						outfile.close();
					}
				}
				else
				{
					print_usage(argv[0]);
					exit(EXIT_FAILURE);
				}
			}
		}

		/* Load Image */
		IplImage* img = cvLoadImage(argv[1], -1);

		/* Test file load success */
		if (!img)
		{
			cerr << "ERROR: Cannot open input file:  \"" << argv[1] << "\"!\n";
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}

		IplImage* gaussian_img = cvCreateImage(cvSize(img->width, img->height), img->depth, img->nChannels);
		IplImage* downsampled_img = cvCreateImage(cvSize((img->width / 2), (img->height / 2)), img->depth, img->nChannels);
		
		GaussianFilter(img, gaussian_img, sigma, radius);
		DecimateImage(gaussian_img, downsampled_img);
		
		/* Write output image */
		cvSaveImage(outfilename.c_str(), downsampled_img);
		CommentPGM(outfilename, DOWNSAMPLER_ID);

		exit(EXIT_SUCCESS);
	}
	catch(cv::Exception& e)
	{
		string error = e.what();
		cerr << "OPENCV ERROR: " << error << "\nEncountered while processing file: " << argv[1] << endl;
		exit(EXIT_FAILURE);
    }
	catch (std::exception &e)
	{
 		string error = e.what();
		cerr << "ERROR: " << error << "\nEncountered while processing file: " << argv[1] << endl;
		exit(EXIT_FAILURE);
	}
	catch (...)
	{
		/* Caught an unkown exception... exit gracefully */
		cerr << "Encountered an unknown exception while processing file: " << argv[1] << endl;
		exit(EXIT_FAILURE);
	}

	return 0;
}
