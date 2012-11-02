#include "TextureDialog/TextureProperties.h"
#include "QtPropertyBrowser/qtpropertymanager.h"
#include "QtPropertyBrowser/qteditorfactory.h"

#include "Render/TextureDescriptor.h"

TextureProperties::TextureProperties(QWidget *parent /* = 0 */)
	: QtGroupBoxPropertyBrowser(parent)//QtTreePropertyBrowser(parent)
	, curTexture(NULL)
	, curTextureDescriptor(NULL)
	, reactOnPropertyChange(true)
{
	// initialize list with string for different comboboxs
	{
		helperPVRFormats.push_back("None", DAVA::FORMAT_INVALID);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA8888), DAVA::FORMAT_RGBA8888);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA5551), DAVA::FORMAT_RGBA5551);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA4444), DAVA::FORMAT_RGBA4444);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGB565), DAVA::FORMAT_RGB565);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_A8), DAVA::FORMAT_A8);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_PVR4), DAVA::FORMAT_PVR4);
		helperPVRFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_PVR2), DAVA::FORMAT_PVR2);


		helperDXTFormats.push_back("None", DAVA::FORMAT_INVALID);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA8888), DAVA::FORMAT_RGBA8888);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA5551), DAVA::FORMAT_RGBA5551);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA4444), DAVA::FORMAT_RGBA4444);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGB565), DAVA::FORMAT_RGB565);
		helperDXTFormats.push_back(DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_A8), DAVA::FORMAT_A8);

		helperWrapModes.push_back("Clamp", DAVA::Texture::WRAP_CLAMP_TO_EDGE);
		helperWrapModes.push_back("Repeat", DAVA::Texture::WRAP_REPEAT);
	}

	// parent widget
	oneForAllParent = new QWidget();

	// property managers
	propertiesGroup = new QtGroupPropertyManager(oneForAllParent);
	propertiesInt = new QtIntPropertyManager(oneForAllParent);
	propertiesBool = new QtBoolPropertyManager(oneForAllParent);
	propertiesEnum = new QtEnumPropertyManager(oneForAllParent);
	propertiesString = new QtStringPropertyManager(oneForAllParent);

	// property editors
	editorInt = new QtSpinBoxFactory(oneForAllParent);
	editorBool = new QtCheckBoxFactory(oneForAllParent);
	editorString = new QtLineEditFactory(oneForAllParent);
	editorEnum = new QtEnumEditorFactory(oneForAllParent);

	// setup property managers with appropriate property editors
	setFactoryForManager(propertiesInt, editorInt);
	setFactoryForManager(propertiesBool, editorBool);
	setFactoryForManager(propertiesEnum, editorEnum);
	setFactoryForManager(propertiesString, editorString);

	// Adding properties

	// groups
	QtProperty* groupPVR = propertiesGroup->addProperty("PVR");
	QtProperty* groupDXT = propertiesGroup->addProperty("DXT");
	QtProperty* groupCommon = propertiesGroup->addProperty("Common");

	// PVR group
	enumPVRFormat = propertiesEnum->addProperty("Format");
	propertiesEnum->setEnumNames(enumPVRFormat, helperPVRFormats.keyList());

	enumBasePVRMipmapLevel = propertiesEnum->addProperty("Base Mipmap level");

	groupPVR->addSubProperty(enumPVRFormat);
	groupPVR->addSubProperty(enumBasePVRMipmapLevel);
	addProperty(groupPVR);

	// DXT group
	enumDXTFormat = propertiesEnum->addProperty("Format");
	propertiesEnum->setEnumNames(enumDXTFormat, helperDXTFormats.keyList());

	enumBaseDXTMipmapLevel = propertiesEnum->addProperty("Base Mipmap level");

	groupDXT->addSubProperty(enumDXTFormat);
	groupDXT->addSubProperty(enumBaseDXTMipmapLevel);
	addProperty(groupDXT);

	// Common group

	// Mip maps
	boolGenerateMipMaps = propertiesBool->addProperty("Generate MipMaps");
	groupCommon->addSubProperty(boolGenerateMipMaps);

	// wrapmode t
	enumWrapModeS = propertiesEnum->addProperty("Wrap mode S");
	propertiesEnum->setEnumNames(enumWrapModeS, helperWrapModes.keyList());
	groupCommon->addSubProperty(enumWrapModeS);

	// wrapmode s
	enumWrapModeT = propertiesEnum->addProperty("Wrap mode T");
	propertiesEnum->setEnumNames(enumWrapModeT, helperWrapModes.keyList());
	groupCommon->addSubProperty(enumWrapModeT);

	addProperty(groupCommon);

	QObject::connect(propertiesEnum, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	QObject::connect(propertiesInt, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	QObject::connect(propertiesBool, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	QObject::connect(propertiesString, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
}

TextureProperties::~TextureProperties()
{
	Save();
	DAVA::SafeRelease(curTexture);
	DAVA::SafeRelease(curTextureDescriptor);

	delete oneForAllParent;
}

void TextureProperties::setTexture(DAVA::Texture *texture, DAVA::TextureDescriptor *descriptor)
{
	reactOnPropertyChange = false;

	Save();
	DAVA::SafeRelease(curTexture);
	DAVA::SafeRelease(curTextureDescriptor);

	curTexture = texture;
	curTextureDescriptor = descriptor;

	if(NULL != curTexture && NULL != curTextureDescriptor)
	{
		DAVA::SafeRetain(curTexture);
		DAVA::SafeRetain(curTextureDescriptor);

		// enable this widget
		setEnabled(true);

		// init mimmap sizes helper
		InitMipMapSizes(curTexture->width, curTexture->height);

		// set loaded descriptor to current properties
		{
			// pvr
			QSize curPVRSize(curTextureDescriptor->pvrCompression.compressToWidth, curTextureDescriptor->pvrCompression.compressToHeight);
			propertiesEnum->setValue(enumPVRFormat, helperPVRFormats.indexV(curTextureDescriptor->pvrCompression.format));
			propertiesEnum->setEnumNames(enumBasePVRMipmapLevel, helperMipMapSizes.keyList());
			propertiesEnum->setValue(enumBasePVRMipmapLevel, helperMipMapSizes.indexV(curPVRSize));

			// dxt
			QSize curDXTSize(curTextureDescriptor->dxtCompression.compressToWidth, curTextureDescriptor->dxtCompression.compressToHeight);
			propertiesEnum->setValue(enumDXTFormat, helperDXTFormats.indexV(curTextureDescriptor->dxtCompression.format));
			propertiesEnum->setEnumNames(enumBaseDXTMipmapLevel, helperMipMapSizes.keyList());
			propertiesEnum->setValue(enumBaseDXTMipmapLevel, helperMipMapSizes.indexV(curDXTSize));

			// mipmap
			propertiesBool->setValue(boolGenerateMipMaps, curTextureDescriptor->generateMipMaps);

			// wrap mode
			propertiesEnum->setValue(enumWrapModeS, helperWrapModes.indexV(curTextureDescriptor->wrapModeS));
			propertiesEnum->setValue(enumWrapModeT, helperWrapModes.indexV(curTextureDescriptor->wrapModeT));
		}
	}
	else
	{
		// no texture - disable this widget
		setEnabled(false);
	}

	reactOnPropertyChange = true;
}

const DAVA::Texture* TextureProperties::getTexture()
{
	return curTexture;
}

const DAVA::TextureDescriptor* TextureProperties::getTextureDescriptor()
{
	return curTextureDescriptor;
}

void TextureProperties::propertyChanged(QtProperty * property)
{
	if(reactOnPropertyChange && NULL != curTextureDescriptor && NULL != curTexture)
	{
		if(property == enumPVRFormat)
		{
			DAVA::PixelFormat newPVRFormat = (DAVA::PixelFormat) helperPVRFormats.value(enumPVRFormat->valueText());
			curTextureDescriptor->pvrCompression.format = newPVRFormat;
		}
		else if(property == enumDXTFormat)
		{
			DAVA::PixelFormat newDXTFormat = (DAVA::PixelFormat) helperDXTFormats.value(enumDXTFormat->valueText());
			curTextureDescriptor->dxtCompression.format = newDXTFormat;
		}
		else if(property == enumBasePVRMipmapLevel)
		{
			QString curSizeStr = enumBasePVRMipmapLevel->valueText();
			QSize size = helperMipMapSizes.value(curSizeStr);

			if(size.width() == curTexture->width || size.height() == curTexture->height)
			{
				curTextureDescriptor->pvrCompression.compressToWidth = 0;
				curTextureDescriptor->pvrCompression.compressToHeight = 0;
			}
			else
			{
				curTextureDescriptor->pvrCompression.compressToWidth = size.width();
				curTextureDescriptor->pvrCompression.compressToHeight = size.height();
			}
		}
		else if(property == enumBaseDXTMipmapLevel)
		{
			QString curSizeStr = enumBaseDXTMipmapLevel->valueText();
			QSize size = helperMipMapSizes.value(curSizeStr);

			if(size.width() == curTexture->width || size.height() == curTexture->height)
			{
				curTextureDescriptor->dxtCompression.compressToWidth = 0;
				curTextureDescriptor->dxtCompression.compressToHeight = 0;
			}
			else
			{
				curTextureDescriptor->dxtCompression.compressToWidth = size.width();
				curTextureDescriptor->dxtCompression.compressToHeight = size.height();
			}
		}
		else if(property == boolGenerateMipMaps)
		{
			curTextureDescriptor->generateMipMaps = (int) propertiesBool->value(boolGenerateMipMaps);
		}
		else if(property == enumWrapModeS)
		{
			curTextureDescriptor->wrapModeS = (DAVA::Texture::TextureWrap) helperWrapModes.value(enumWrapModeS->valueText());
		}
		else if(property == enumWrapModeS)
		{
			curTextureDescriptor->wrapModeT = (DAVA::Texture::TextureWrap) helperWrapModes.value(enumWrapModeT->valueText());
		}

		emit propertyChanged();
	}
}

void TextureProperties::Save()
{
	if(NULL != curTextureDescriptor)
	{
		curTextureDescriptor->Save();
	}
}

void TextureProperties::InitMipMapSizes(int baseWidth, int baseHeight)
{
	int level = 0;

	helperMipMapSizes.clear();
	while(baseWidth > 1 && baseHeight > 1)
	{
		QSize size(baseWidth, baseHeight);
		QString shownKey;

		shownKey.sprintf("%d - %dx%d", level, baseWidth, baseHeight);
		helperMipMapSizes.push_back(shownKey, size);

		level++;
		baseWidth = baseWidth >> 1;
		baseHeight = baseHeight >> 1;
	}
}
