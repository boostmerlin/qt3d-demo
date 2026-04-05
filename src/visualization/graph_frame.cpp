//
// Created by merlin
//

#include "graph_frame.h"

GraphFrame::GraphFrame(QWidget *parent) : QFrame(parent) {
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
    this->setFrameStyle(Panel);
    this->setLineWidth(1);

    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignTop);
    m_toolBar = new QToolBar();
    m_toolBar->setVisible(false);
    m_layout->addWidget(m_toolBar);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    setLayout(m_layout);
}

QSize GraphFrame::sizeHint() const {
    return {600, 0};
}

void GraphFrame::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);
    onResized(event);
}
