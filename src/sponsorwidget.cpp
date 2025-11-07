/*
 * iDescriptor: A free and open-source idevice management tool.
 *
 * Copyright (C) 2025 Uncore <https://github.com/uncor3>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "sponsorwidget.h"
#include "sponsorappcard.h"
#include <QLabel>
#include <QVBoxLayout>

SponsorWidget::SponsorWidget(QWidget *parent) : QWidget(parent)
{
    setLayout(new QVBoxLayout(this));
    QLabel *sponsorTitle = new QLabel("Would you like to sponsor us?");
    sponsorTitle->setStyleSheet("font-weight: bold; font-size: 16pt;");
    sponsorTitle->setAlignment(Qt::AlignCenter);

    QLabel *sponsorDesc =
        new QLabel("This app is open-source and free to use. "
                   "And in order to keep it that way, we rely on donations. "
                   "Consider becoming a sponsor to support "
                   "and promote your app/brand here");
    sponsorDesc->setStyleSheet("font-size: 10pt;");
    sponsorDesc->setWordWrap(true);
    layout()->addWidget(sponsorTitle);
    layout()->addWidget(sponsorDesc);
    QLabel *sponsorIconLabel = new QLabel("Example:");
    layout()->addWidget(sponsorIconLabel);
    SponsorAppCard *card = new SponsorAppCard(this);
    layout()->addWidget(card);
    layout()->setAlignment(card, Qt::AlignCenter);
}
