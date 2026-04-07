#include "property_panel.h"

#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

#include "property_panel/property_section.h"
#include "property_panel/sections/extrude_section.h"
#include "property_panel/sections/line_section.h"
#include "property_panel/sections/object_section.h"
#include "property_panel/sections/polygon_section.h"
#include "property_panel/sections/primitive_dimensions_section.h"
#include "property_panel/sections/transform_section.h"

PropertyPanel::PropertyPanel(SceneController *sceneController, QWidget *parent)
    : QWidget(parent)
{
    auto *wrapperLayout = new QVBoxLayout(this);
    wrapperLayout->setContentsMargins(12, 12, 12, 12);
    wrapperLayout->setSpacing(10);

    auto *title = new QLabel(tr("Properties"), this);
    title->setStyleSheet("font-weight: 600; font-size: 13px; color: #334155;");
    wrapperLayout->addWidget(title);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");
    wrapperLayout->addWidget(scrollArea, 1);

    auto *scrollContent = new QWidget(scrollArea);
    auto *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(10);
    scrollArea->setWidget(scrollContent);

    m_emptyStateLabel = new QLabel(tr("No object selected"), scrollContent);
    m_emptyStateLabel->setAlignment(Qt::AlignCenter);
    m_emptyStateLabel->setStyleSheet("color: #64748b; padding: 24px 12px;");
    scrollLayout->addWidget(m_emptyStateLabel);

    m_propertyContentWidget = new QWidget(scrollContent);
    auto *contentLayout = new QVBoxLayout(m_propertyContentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(14);
    scrollLayout->addWidget(m_propertyContentWidget);

    m_objectSection = new ObjectSection(sceneController, m_propertyContentWidget);
    auto *lineSection = new LineSection(sceneController, m_propertyContentWidget);
    auto *transformSection = new TransformSection(sceneController, m_propertyContentWidget);
    auto *dimensionsSection = new PrimitiveDimensionsSection(sceneController, m_propertyContentWidget);
    auto *polygonSection = new PolygonSection(sceneController, m_propertyContentWidget);
    auto *extrudeSection = new ExtrudeSection(sceneController, m_propertyContentWidget);

    m_sections = {m_objectSection, lineSection, transformSection, dimensionsSection, polygonSection, extrudeSection};
    for (auto *section : m_sections) {
        contentLayout->addWidget(section);
        section->setVisible(false);
    }
    contentLayout->addStretch(1);

    connect(m_objectSection, &ObjectSection::parentChangeRequested, this, &PropertyPanel::parentChangeRequested);
    connect(m_objectSection, &ObjectSection::removeRequested, this, &PropertyPanel::removeRequested);

    m_propertyContentWidget->setVisible(false);
    m_emptyStateLabel->setVisible(true);
}

void PropertyPanel::setObjects(const QList<SceneObject *> &objects)
{
    m_objects = objects;
    for (auto *section : m_sections) {
        section->setObjects(objects);
    }
}

void PropertyPanel::setCurrentObject(SceneObject *object)
{
    m_currentObject = object;
    m_propertyContentWidget->setVisible(object != nullptr);
    m_emptyStateLabel->setVisible(object == nullptr);
    for (auto *section : m_sections) {
        section->setCurrentObject(object);
    }
}

void PropertyPanel::refreshObject(SceneObject *object)
{
    if (!object) {
        return;
    }
    for (auto *section : m_sections) {
        section->refreshObject(object);
    }
}
