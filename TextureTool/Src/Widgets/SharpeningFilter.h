#ifndef SHARPENINGFILTER_H
#define SHARPENINGFILTER_H

#include <QDialog>
#include "ui_SharpeningFilter.h"

#define KERNELMATRIX_WIDTH  3
#define KERNELMATRIX_HEIGHT 3
#define KERNELMATRIX_LENGTH (KERNELMATRIX_WIDTH * KERNELMATRIX_HEIGHT) 


class KernelPreset
{
public:
	KernelPreset() { mFilter = new int[KERNELMATRIX_LENGTH]; }
	~KernelPreset() 
	{
		delete[] mFilter;
	}
	// Override all presets are to override the setup function and fill in the variables
	virtual void                Setup() = 0;

	// Override all presets are to override the GetName and return the filter name
	virtual const char*         GetName() = 0;

	int*                        GetFilterList() { return mFilter; }

	int                         GetDivider() { return mDivider; }
protected:
	int*                        mFilter;
	int                         mDivider;
};

struct KernelFilter 
{
	// Don't store the divider, calculate it before updating this array
	float                       mFilter[KERNELMATRIX_LENGTH]; 
};

class SharpeningFilter : public QDialog
{
	Q_OBJECT

public:
	SharpeningFilter(QWidget *parent = 0);
	~SharpeningFilter();

	// Opens the Sharpening filter editor
	void                        Configure();

	void                        SetSharpeningEnabled(bool pSharpeningEnabled) { mSharpeningEnabled = pSharpeningEnabled; }

	// Applies the sharpening filter to the surface
	void                        ApplyKernelToSurface(Surface* pSurface);

	// Pushes the values in the editor to the mKernelFilter variable
	void                        UpdateKernelFilter();

	// Checkbox for whether to apply the filter on the first surface/mipmaps
	bool                        GetOnlyApplyOnMipmaps() { return mUi.mOnlyOnMipmapsCheckbox->isChecked(); }

	// Updates the preset combobox with all the available presets
	void                        UpdatePresetCB();
		
	bool                        GetSharpeningEnabled() { return mSharpeningEnabled; }

private slots:
	void                        OnPresetCBIndexChanged();

private:
	// All the possible presets 
	std::vector<KernelPreset*>  mKernelPresetList;

	std::vector<QLineEdit*>     mLineEditList;
	// The current kernel filter values
	KernelFilter                mKernelFilter;

	// If false, the apply function is skipped
	bool                        mSharpeningEnabled;

	Ui::SharpeningFilter        mUi;
};

#endif // SHARPENINGFILTER_H
