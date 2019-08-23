/*
 * Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     lq <longqi_cm@deepin.com>
 *
 * Maintainer: lq <longqi_cm@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "microphonepage.h"
#include "modules/sound/soundmodel.h"

#include "widgets/switchwidget.h"
#include "widgets/titledslideritem.h"
#include "widgets/dccslider.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

using namespace dcc::sound;
using namespace dcc::widgets;

#include "modules/sound/soundmodel.h"

#include "widgets/switchwidget.h"
#include "widgets/titledslideritem.h"
#include "widgets/dccslider.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

using namespace dcc::sound;
using namespace dcc::widgets;

using namespace DCC_NAMESPACE::sound;

MicrophonePage::MicrophonePage(QWidget *parent)
    : QWidget(parent)
    , m_layout(new QVBoxLayout)
    , m_sw(new SwitchWidget)
    , m_inputSlider(new TitledSliderItem("Input Volume"))

{
    setMinimumWidth(400);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(new QLabel(tr("Speaker")));
    hlayout->addStretch(1);
    hlayout->addWidget(m_sw);

    m_layout->addLayout(hlayout);
    m_layout->addStretch(1);
    setLayout(m_layout);
}


MicrophonePage::~MicrophonePage()
{
    if (m_feedbackSlider)
        m_feedbackSlider->disconnect(m_conn);
    m_feedbackSlider->deleteLater();
}

void MicrophonePage::setModel(SoundModel *model)
{
    m_model = model;

    m_sw->setChecked(m_model->microphoneOn());
    connect(m_sw, &SwitchWidget::checkedChanged, this, &MicrophonePage::requestSwitchMicrophone);
    connect(m_sw, &SwitchWidget::checkedChanged, this, [ = ](bool checked) {
        if (checked) {
            //enable slider
        } else {
            //disable slider
        }
    });
    connect(m_model, &SoundModel::microphoneOnChanged, m_sw, &SwitchWidget::setChecked);

    initSlider();
}

void MicrophonePage::initSlider()
{
    DCCSlider *slider = m_inputSlider->slider();
    slider->setRange(0, 100);
    slider->setType(DCCSlider::Vernier);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setTickInterval(1);
    slider->setSliderPosition(m_model->microphoneVolume() * 100);
    slider->setPageStep(1);

    auto slotfunc1 = [ = ](int pos) {
        double val = pos / 100.0;
        Q_EMIT requestSetMicrophoneVolume(val);
    };
    connect(slider, &DCCSlider::valueChanged, this, slotfunc1);
    connect(slider, &DCCSlider::sliderMoved, this, slotfunc1);

    m_layout->insertWidget(1, m_inputSlider);


    m_feedbackSlider = (new TitledSliderItem("Feedback Volume"));
    DCCSlider *slider2 = m_feedbackSlider->slider();
    slider2->setRange(0, 100);
    slider2->setEnabled(false);
    slider2->setType(DCCSlider::Vernier);
    slider2->setTickPosition(QSlider::TicksBelow);
    slider2->setTickInterval(1);
    slider2->setPageStep(1);

    m_conn = connect(m_model, &SoundModel::microphoneFeedbackChanged, [ = ](double vol2) {
        slider2->setSliderPosition(vol2 * 100);
    });

    m_layout->insertWidget(2, m_feedbackSlider);
}
