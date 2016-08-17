#include "TT_PCH.h"

// KernelPreset implementation
class SharpenSoft:public KernelPreset
{
public:
	virtual void Setup() override
	{
		int filter[] = { -1, -1, -1,
						 -1,  16, -1,
						 -1, -1, -1 };
		memcpy(mFilter, filter, sizeof(int)*KERNELMATRIX_LENGTH);
		mDivider = 8;
	}
	virtual const char* GetName() override { return "Sharpen Soft"; }
};

class SharpenMedium:public KernelPreset
{
public:
	virtual void Setup() override
	{
		int filter[] = {  0, -2,  0,
						 -2, 11, -2,
						  0, -2,  0};
		memcpy(mFilter, filter, sizeof(int)*KERNELMATRIX_LENGTH);
		mDivider = 3;
	}
	virtual const char* GetName() override { return "Sharpen Medium"; }
};

class SharpenStrong:public KernelPreset
{
public:
	virtual void Setup() override
	{
		int filter[] = { -1, -1, -1,
						 -1,  9, -1,
						 -1, -1, -1 };
		memcpy(mFilter, filter, sizeof(int)*KERNELMATRIX_LENGTH);
		mDivider = 1;
	}
	virtual const char* GetName() override { return "Sharpen Strong"; }
};



//SharpeningFilter implementation 

SharpeningFilter::SharpeningFilter(QWidget *parent)
	: QDialog(parent),
	  mSharpeningEnabled(false)
{
	mUi.setupUi(this);

	mLineEditList.push_back(mUi.mLineEditNW);
	mLineEditList.push_back(mUi.mLineEditN);
	mLineEditList.push_back(mUi.mLineEditNE);

	mLineEditList.push_back(mUi.mLineEditW);
	mLineEditList.push_back(mUi.mLineEditCenter);
	mLineEditList.push_back(mUi.mLineEditE);

	mLineEditList.push_back(mUi.mLineEditSW);
	mLineEditList.push_back(mUi.mLineEditS);
	mLineEditList.push_back(mUi.mLineEditSE);

	UpdateKernelFilter();

	for (int i = 0; i < mLineEditList.size(); i++)
	{
		QLineEdit* lineEdit = mLineEditList[i];
		lineEdit->setValidator(new QIntValidator(lineEdit));
	}

	connect(mUi.mOkBtn, SIGNAL(clicked()), SLOT(accept()));

	SharpenSoft* sharpenSoft = new SharpenSoft();
	sharpenSoft->Setup();
	mKernelPresetList.push_back(sharpenSoft);

	SharpenMedium* sharpenMedium = new SharpenMedium();
	sharpenMedium->Setup();
	mKernelPresetList.push_back(sharpenMedium);


	SharpenStrong* sharpenStrong = new SharpenStrong();
	sharpenStrong->Setup();
	mKernelPresetList.push_back(sharpenStrong);

	UpdatePresetCB();


	connect(mUi.mPresetsCB, SIGNAL(currentIndexChanged(int)), SLOT(OnPresetCBIndexChanged()));
}

SharpeningFilter::~SharpeningFilter()
{
	for (int i = 0; i < mKernelPresetList.size(); i++) 
	{
		SAFE_DELETE(mKernelPresetList[i]);
	}
	mKernelPresetList.clear();
}

void SharpeningFilter::Configure()
{
	exec();
	UpdateKernelFilter();
}

void SharpeningFilter::ApplyKernelToSurface(Surface* pSurface)
{
	// Copy the surface onto a new surface, so when we read it will not read any converted pixels
	Surface* cleanSurface = new Surface();

	// Assign the existing surface onto the clean surface
	cleanSurface->Assign(pSurface);

	QRgb* cleanColorData = (QRgb*)cleanSurface->GetData();

	const int surfaceWidth  = pSurface->GetWidth();
	const int surfaceHeight = pSurface->GetHeight();

	// Apply the converted data directly to the assigned surface
	QRgb* convertedColorData = (QRgb*)pSurface->GetData();

	// Loop through every pixel
	for (int x = 0; x < surfaceWidth; x++)
	{
		for (int y = 0; y < surfaceHeight; y++)
		{
			const int pixelIndex = x + y*surfaceWidth;

			// Get the clean pixel color
			QColor cleanPixelColor((QRgb)cleanColorData[pixelIndex]);
			if (cleanPixelColor.alphaF() <= 0.0f)
			{
				continue;
			}

			float convertedRed =0;
			float convertedGreen =0;
			float convertedBlue =0;

			// Loop trough the kernel array
			for (int kernelIndexX = -1; kernelIndexX <= 1; kernelIndexX++) 
			{
				for (int kernelIndexY = -1; kernelIndexY <= 1; kernelIndexY++)
				{
					// The image pixel with the kernel offset
					int textureKernelIndexX = x + kernelIndexX;
					int textureKernelIndexY = y + kernelIndexY;

					// Don't go outside the texture 
					textureKernelIndexX = min(max(textureKernelIndexX, 0), surfaceWidth - 1); 
					textureKernelIndexY = min(max(textureKernelIndexY, 0), surfaceHeight - 1);

					QColor kernelColor = cleanColorData[textureKernelIndexX + textureKernelIndexY * surfaceWidth];

					// The index to find the kernel value in the array
					const int kernelMatrixIndex = (kernelIndexX + 1) + (kernelIndexY + 1) * KERNELMATRIX_WIDTH;

					assert(kernelMatrixIndex < KERNELMATRIX_LENGTH);

					const float kernelValue = mKernelFilter.mFilter[kernelMatrixIndex];

					convertedRed += kernelValue * kernelColor.redF();
					convertedGreen += kernelValue * kernelColor.greenF();
					convertedBlue += kernelValue * kernelColor.blueF();
				}
			}
			QColor convertedColor;
			convertedColor.setRedF(max(min(1.0f, convertedRed), 0));
			convertedColor.setGreenF(max(min(1.0f, convertedGreen), 0));
			convertedColor.setBlueF(max(min(1.0f, convertedBlue), 0));
			convertedColor.setAlphaF(cleanPixelColor.alphaF());
			convertedColorData[pixelIndex] = convertedColor.rgb();
		}
	}
	delete cleanSurface;
}

void SharpeningFilter::UpdateKernelFilter()
{
	// Update the kernel filter values to the values that are in the textboxes
	assert(mLineEditList.size() == KERNELMATRIX_LENGTH);
	float divider = (float)mUi.mDividerTB->text().toInt();

	for (int i = 0; i < mLineEditList.size(); i++)
	{
		QLineEdit* textBox = mLineEditList[i];
		// Apply the divider here
		float filterValue = (float)textBox->text().toInt() / divider;

		mKernelFilter.mFilter[i] = filterValue;
	}
}

void SharpeningFilter::UpdatePresetCB()
{
	//Make sure no signals are emitted during the clearing or adding
	mUi.mPresetsCB->blockSignals(true);
	mUi.mPresetsCB->clear();
	for (int i = 0; i < mKernelPresetList.size(); i++)
	{
		KernelPreset* preset = mKernelPresetList[i];

		QVariant customData;
		customData.setValue<void*>(preset);

		mUi.mPresetsCB->addItem(preset->GetName(), customData);
	}
	mUi.mPresetsCB->blockSignals(false);
}

void SharpeningFilter::OnPresetCBIndexChanged()
{
	assert(mLineEditList.size() == KERNELMATRIX_LENGTH);

	KernelPreset* preset = (KernelPreset*)mUi.mPresetsCB->currentData().value<void*>();
	int* filter = preset->GetFilterList();
	for (int i = 0; i < mLineEditList.size(); i++)
	{
		QLineEdit* textBox = mLineEditList[i];
		textBox->setText(QString::number(filter[i]));
	}
	mUi.mDividerTB->setText(QString::number(preset->GetDivider()));
}


