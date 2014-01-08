/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include <QDir>
#include <QDirIterator>

#include "MaterialEditor.h"
#include "ui_materialeditor.h"

#include "Main/mainwindow.h"
#include "Main/QtUtils.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataIntrospection.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataInspDynamic.h"
#include "Tools/QtWaitDialog/QtWaitDialog.h"

MaterialEditor::MaterialEditor(QWidget *parent /* = 0 */)
: QDialog(parent)
, ui(new Ui::MaterialEditor)
, curMaterial(NULL)
, templatesScaned(false)
{
	ui->setupUi(this);
	setWindowFlags(WINDOWFLAG_ON_TOP_OF_APPLICATION);

	// global scene manager signals
	QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(sceneActivated(SceneEditor2 *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(sceneDeactivated(SceneEditor2 *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(SelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)), this, SLOT(sceneSelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)));

	// ui signals
	QObject::connect(ui->materialTree, SIGNAL(clicked(const QModelIndex &)), this, SLOT(materialClicked(const QModelIndex &)));
	QObject::connect(ui->materialTree, SIGNAL(clicked(const QModelIndex &)), this, SLOT(materialClicked(const QModelIndex &)));

	// material properties
	QObject::connect(ui->materialProperty, SIGNAL(PropertyEdited(const QModelIndex &)), this, SLOT(OnPropertyEdited(const QModelIndex &)));
	QObject::connect(ui->templateBox, SIGNAL(activated(int)), this, SLOT(OnTemplateChanged(int)));

	ui->materialTree->setDragEnabled(true);
	ui->materialTree->setAcceptDrops(true);
	ui->materialTree->setDragDropMode(QAbstractItemView::DragDrop);

	ui->materialProperty->SetEditTracking(true);

	posSaver.Attach(this);
	posSaver.LoadState(ui->splitter);
	posSaver.LoadState(ui->splitter_2);

	DAVA::VariantType v1 = posSaver.LoadValue("splitPosProperties");
	DAVA::VariantType v2 = posSaver.LoadValue("splitPosTexttures");
	if(v1.GetType() == DAVA::VariantType::TYPE_INT32) ui->materialProperty->header()->resizeSection(0, v1.AsInt32());
	if(v2.GetType() == DAVA::VariantType::TYPE_INT32) ui->materialTexture->header()->resizeSection(0, v2.AsInt32());
}

MaterialEditor::~MaterialEditor()
{ 
	DAVA::VariantType v1(ui->materialProperty->header()->sectionSize(0));
	DAVA::VariantType v2(ui->materialTexture->header()->sectionSize(0));
	posSaver.SaveValue("splitPosProperties", v1);
	posSaver.SaveValue("splitPosTexttures", v2);

	posSaver.SaveState(ui->splitter);
	posSaver.SaveState(ui->splitter_2);
}

void MaterialEditor::SetCurMaterial(DAVA::NMaterial *material)
{
	curMaterial = material;

	// Don't remove properties immediately. Just extract them and remove later
	// this should be done, because we want to remove all properties when propertyEdited signal emited
	{
		QtPropertyData *propRoot = ui->materialProperty->GetRootProperty();
		while(propRoot->ChildCount() > 0)
		{
			QtPropertyData *child = propRoot->ChildGet(0);
			propRoot->ChildExtract(child);

			child->deleteLater();
		}

		propRoot = ui->materialTexture->GetRootProperty();
		while(propRoot->ChildCount() > 0)
		{
			QtPropertyData *child = propRoot->ChildGet(0);
			propRoot->ChildExtract(child);

			child->deleteLater();
		}
	}

	if(NULL != material)
	{
		FillMaterialProperties(material);
		FillMaterialTextures(material);

		// set current template
		DAVA::FilePath curMaterialTemplate = material->GetMaterialTemplate()->name.c_str();
		int curIndex = templates.indexOf(curMaterialTemplate);

		if(-1 != curIndex)
		{
			ui->templateBox->setCurrentIndex(curIndex);
		}
		else
		{
			ui->templateBox->setCurrentIndex(0);
		}
	}
}

void MaterialEditor::sceneActivated(SceneEditor2 *scene)
{
	if(isVisible())
	{
		ui->materialTree->SetScene(scene);
	}
}

void MaterialEditor::sceneDeactivated(SceneEditor2 *scene)
{ 
	ui->materialTree->SetScene(NULL);
	SetCurMaterial(NULL);
}

void MaterialEditor::sceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected)
{ }

void MaterialEditor::materialClicked(const QModelIndex &index)
{
	DAVA::NMaterial *material = ui->materialTree->GetMaterial(index);
	SetCurMaterial(material);
}

void MaterialEditor::showEvent(QShowEvent * event)
{
	sceneActivated(QtMainWindow::Instance()->GetCurrentScene());

	if(!templatesScaned)
	{
		ScanTemplates();
	}
}

void MaterialEditor::FillMaterialProperties(DAVA::NMaterial *material)
{
	const DAVA::InspInfo *info = material->GetTypeInfo();
	const DAVA::InspMember *materialProperties = info->Member("materialProperties");
	const DAVA::InspMember *materialFlags = info->Member("materialSetFlags");

	// fill material flags
	if(NULL != materialFlags)
	{
		const DAVA::InspMemberDynamic* dynamicInsp = materialFlags->Dynamic();

		if(NULL != dynamicInsp)
		{
			DAVA::InspInfoDynamic *dynamicInfo = dynamicInsp->GetDynamicInfo();

			size_t count = dynamicInfo->MembersCount(material); // this function can be slow
			for(size_t i = 0; i < count; ++i)
			{
				QtPropertyDataInspDynamic *dynamicMember = new QtPropertyDataInspDynamic(material, dynamicInfo, i);
				ui->materialProperty->AppendProperty(dynamicInfo->MemberName(material, i), dynamicMember);
			}
		}
	}

	// fill material properties
	if(NULL != materialProperties)
	{
		const DAVA::InspMemberDynamic* dynamicInsp = materialProperties->Dynamic();

		if(NULL != dynamicInsp)
		{
			DAVA::InspInfoDynamic *dynamicInfo = dynamicInsp->GetDynamicInfo();

			size_t count = dynamicInfo->MembersCount(material); // this function can be slow
			for(size_t i = 0; i < count; ++i)
			{
				int memberFlags = dynamicInfo->MemberFlags(material, i);
				QtPropertyDataInspDynamic *dynamicMember = new QtPropertyDataInspDynamic(material, dynamicInfo, i);

				// self property
				if(memberFlags & DAVA::I_EDIT)
				{
					QtPropertyToolButton* btn = dynamicMember->AddButton();
					btn->setIcon(QIcon(":/QtIcons/cminus.png"));
					btn->setIconSize(QSize(14, 14));
					QObject::connect(btn, SIGNAL(clicked()), this, SLOT(OnRemProperty()));

					// isn't set in parent or shader
					if(!(memberFlags & DAVA::I_VIEW) && !(memberFlags & DAVA::I_SAVE))
					{
						dynamicMember->SetBackground(QBrush(QColor(255, 0, 0, 10)));
					}
				}
				// not self property (is set in parent or shader)
				else
				{
					dynamicMember->SetEnabled(false);

					QtPropertyToolButton* btn = dynamicMember->AddButton();
					btn->setIcon(QIcon(":/QtIcons/cplus.png"));
					btn->setIconSize(QSize(14, 14));
					QObject::connect(btn, SIGNAL(clicked()), this, SLOT(OnAddProperty()));

					dynamicMember->SetBackground(QBrush(QColor(0, 0, 0, 10)));

					// required by shader
					//if(!(memberFlags & DAVA::I_VIEW) && (memberFlags & DAVA::I_SAVE))
					//{	}
				}

				ui->materialProperty->AppendProperty(dynamicInfo->MemberName(material, i), dynamicMember);
			}
		}
	}
}

void MaterialEditor::FillMaterialTextures(DAVA::NMaterial *material)
{
	const DAVA::InspInfo *info = material->GetTypeInfo();
	const DAVA::InspMember *materialTextures = info->Member("textures");

	// fill own material textures
	if(NULL != materialTextures)
	{
		QtPropertyData *data = QtPropertyDataIntrospection::CreateMemberData(material, materialTextures);
		while(0 != data->ChildCount())
		{
			QtPropertyData *c = data->ChildGet(0);
			data->ChildExtract(c);

			ui->materialTexture->AppendProperty(c->GetName(), c);
		}

		delete data;
	}
}

void MaterialEditor::ScanTemplates()
{
	QString materialsPath = DAVA::FilePath("~res:/Materials/Legacy/").GetAbsolutePathname().c_str();

	QtWaitDialog waitDlg;
	waitDlg.Show("Scanning material templates", "", true, false);

	templates.clear();
	ui->templateBox->clear();

	// add unknown template
	templates.append("");
	ui->templateBox->addItem("Unknown", 0);

	// scan for known templates
	int i = 1;
	QDir materialDir(materialsPath);
	materialDir.setNameFilters(QStringList("*.material"));

	QDirIterator iterator(materialDir.absolutePath(), QDirIterator::Subdirectories);
	while(iterator.hasNext()) 
	{
		iterator.next();
		QFileInfo fInfo = iterator.fileInfo();

		if(!fInfo.isDir()) 
		{
			waitDlg.SetMessage(fInfo.absoluteFilePath());

			DAVA::FilePath templatePath = fInfo.absoluteFilePath().toAscii().data();
			templates.append(templatePath.GetFrameworkPath());

			ui->templateBox->addItem(fInfo.completeBaseName(), i++);
		}
	}

	waitDlg.Reset();
	templatesScaned = true;
}

void MaterialEditor::OnAddProperty()
{
	QtPropertyToolButton *btn = dynamic_cast<QtPropertyToolButton *>(QObject::sender());
	
	if(NULL != btn && NULL != curMaterial)
	{
		QtPropertyDataInspDynamic *data = dynamic_cast<QtPropertyDataInspDynamic *>(btn->GetPropertyData());
		if(NULL != data)
		{
			data->SetValue(data->GetValue(), QtPropertyData::VALUE_EDITED);

			// reload material properties
			SetCurMaterial(curMaterial);
		}
	}
}

void MaterialEditor::OnRemProperty()
{
	QtPropertyToolButton *btn = dynamic_cast<QtPropertyToolButton *>(QObject::sender());

	if(NULL != btn && NULL != curMaterial)
	{
		QtPropertyDataInspDynamic *data = dynamic_cast<QtPropertyDataInspDynamic *>(btn->GetPropertyData());
		if(NULL != data)
		{
			data->SetValue(QVariant(), QtPropertyData::VALUE_EDITED);

			// reload material properties
			SetCurMaterial(curMaterial);
		}
	}
}

void MaterialEditor::OnAddTexture()
{
	
}

void MaterialEditor::OnRemTexture()
{

}

void MaterialEditor::OnTemplateChanged(int index)
{
	if(NULL != curMaterial && index > 0 && index < templates.size())
	{
		DAVA::FilePath newTemplateName = templates[index];
		DAVA::NMaterialHelper::SwitchTemplate(curMaterial, DAVA::FastName(newTemplateName.GetFrameworkPath().c_str()));
	}

	SetCurMaterial(curMaterial);
}

void MaterialEditor::OnPropertyEdited(const QModelIndex &index)
{
	// reload material properties
	SetCurMaterial(curMaterial);
}
