#pragma once

#include "pch.hpp"
#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QElapsedTimer>
#include <QScrollBar>
#include <QListWidget>

namespace cp {
	class StringField : public QWidget {
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif

	public:
			StringField(std::string* _value, const std::string& _fieldName, QWidget* parent = nullptr)
				: QWidget(parent), valuePtr(_value)
			{
				QHBoxLayout* layout = new QHBoxLayout(this);
				layout->setContentsMargins(0, 0, 0, 0);
				label = new QLabel(QString::fromStdString(_fieldName), this);
				lineEdit = new QLineEdit(QString::fromStdString(*valuePtr), this);
				lineEdit->setStyleSheet("QLineEdit { padding: 2px 4px; background-color: #1A1F2B; }");
				layout->addWidget(label);
				layout->addWidget(lineEdit);
				setLayout(layout);
				QObject::connect(lineEdit, &QLineEdit::textChanged, this, &StringField::OnTextChanged);
			}

			void OnTextChanged(const QString& _text) {
				*valuePtr = _text.toStdString();
			}

			virtual ~StringField() {
				delete label;
				delete lineEdit;
			}
	
		protected:
			QLabel* label = nullptr;
			QLineEdit* lineEdit = nullptr;
			std::string* valuePtr = nullptr;
	};

	class ModernCheckBoxWidget : public QWidget {
	public:
		ModernCheckBoxWidget(const QString& text = QString(), QWidget* parent = nullptr)
			: QWidget(parent), label(text)
		{
			setCursor(Qt::PointingHandCursor);
			setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
			setMinimumHeight(22);

			timer = new QTimer(this);
			QObject::connect(timer, &QTimer::timeout, [this]() { OnAnimateTick(); });

			// état initial
			checked = false;
			animProgress = 0.0;
		}

		~ModernCheckBoxWidget() override = default;

		// API d'accès
		void SetChecked(bool c) noexcept {
			if (checked == c) return;
			StartAnimation(c);
			checked = c;
			if (onChanged) onChanged(checked);
		}

		bool IsChecked() const noexcept { return checked; }

		void SetText(const std::string& text) noexcept {
			label = QString::fromStdString(text);
			update();
		}

		std::string GetText() const noexcept {
			return label.toStdString();
		}

		void SetOnCheckedChangedListener(const std::function<void(bool)>& listener) noexcept {
			onChanged = listener;
		}

		// taille préférée
		QSize sizeHint() const override {
			QFontMetrics fm(font());
			int textW = fm.horizontalAdvance(label);
			int height = 26;
			int width = 48 + textW;
			return QSize(width, height);
		}

	protected:
		void paintEvent(QPaintEvent* /*event*/) override {
			QPainter p(this);
			p.setRenderHint(QPainter::Antialiasing);

			// couleurs
			QColor bg = Qt::transparent; // fond de la zone
			QColor trackOff = QColor("#2E3440");
			QColor trackOn = QColor("#A66BFF");
			QColor knob = QColor("#FFFFFF");
			QColor textColor = QColor("#D0D3DC");

			// géométrie
			const int trackH = 16;
			const int trackW = 32;
			const int knobDiam = 14;
			const int marginLeft = 0;
			const int leftPadding = 0;
			const int spacing = 8;

			int yCenter = height() / 2;
			int trackX = marginLeft;
			int trackY = yCenter - trackH / 2;

			// draw background (transparent here, host controls actual background)
			p.fillRect(rect(), bg);

			// track rect
			QRectF trackRect(trackX, trackY, trackW, trackH);
			qreal radius = trackH / 2.0;

			// interpolated color between off/on
			QColor currentTrack = InterpolateColor(trackOff, trackOn, animProgress);

			// border
			p.setPen(Qt::NoPen);
			p.setBrush(currentTrack);
			p.drawRoundedRect(trackRect, radius, radius);

			// knob position lerp
			qreal knobXOff = trackX + 1 + animProgress * (trackW - knobDiam - 2);
			qreal knobY = yCenter - knobDiam / 2.0;

			// shadow
			QPainterPath shadowPath;
			shadowPath.addEllipse(QRectF(knobXOff - 1.0, knobY + 1.0, knobDiam + 2.0, knobDiam + 2.0));
			p.setBrush(QColor(0, 0, 0, 80));
			p.setPen(Qt::NoPen);
			p.drawPath(shadowPath);

			// knob
			p.setBrush(knob);
			p.setPen(Qt::NoPen);
			p.drawEllipse(QRectF(knobXOff, knobY, knobDiam, knobDiam));

			// label (à droite)
			p.setPen(textColor);
			QFont f = font();
			f.setPointSizeF(f.pointSizeF());
			p.setFont(f);

			int textX = trackX + trackW + spacing;
			int textY = 0;
			QRect textRect(textX, textY, width() - textX, height());
			p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, label);
		}

		void mousePressEvent(QMouseEvent* event) override {
			if (event->button() != Qt::LeftButton) {
				QWidget::mousePressEvent(event);
				return;
			}
			// toggle
			SetChecked(!checked);
		}

	private:
		// animation manuelle (pas de Q_PROPERTY pour éviter moc)
		void StartAnimation(bool targetChecked) {
			animationStart = animProgress;
			animationTarget = targetChecked ? 1.0 : 0.0;
			animStartTime = std::chrono::steady_clock::now();
			animDurationMs = 160;
			if (!timer->isActive()) timer->start(16);
		}

		void OnAnimateTick() {
			using namespace std::chrono;
			auto now = steady_clock::now();
			auto elapsed = duration_cast<milliseconds>(now - animStartTime).count();
			if (elapsed >= animDurationMs) {
				animProgress = animationTarget;
				timer->stop();
				update();
				return;
			}
			double t = static_cast<double>(elapsed) / static_cast<double>(animDurationMs);
			// easeOutCubic
			double eased = 1.0 - std::pow(1.0 - t, 3.0);
			animProgress = animationStart + (animationTarget - animationStart) * eased;
			update();
		}

		static QColor InterpolateColor(const QColor& a, const QColor& b, qreal t) {
			int r = static_cast<int>(a.red() + (b.red() - a.red()) * t);
			int g = static_cast<int>(a.green() + (b.green() - a.green()) * t);
			int bl = static_cast<int>(a.blue() + (b.blue() - a.blue()) * t);
			int alpha = static_cast<int>(a.alpha() + (b.alpha() - a.alpha()) * t);
			return QColor(r, g, bl, alpha);
		}

	private:
		QString label;
		bool checked = false;

		// animation state
		qreal animProgress = 0.0; // 0..1
		qreal animationStart = 0.0;
		qreal animationTarget = 0.0;
		std::chrono::steady_clock::time_point animStartTime;
		int animDurationMs = 160;
		QTimer* timer = nullptr;

		std::function<void(bool)> onChanged;
	};

	class ModernComboBoxWidget : public QWidget {
	public:
		ModernComboBoxWidget(QWidget* parent = nullptr)
			: QWidget(parent)
		{
			setCursor(Qt::PointingHandCursor);
			setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
			setMinimumHeight(26);

			popup = new QListWidget(nullptr);
			popup->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
			popup->setFocusPolicy(Qt::NoFocus);
			popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			popup->setSelectionMode(QAbstractItemView::SingleSelection);
			popup->setSpacing(0);
			popup->setFrameShape(QFrame::NoFrame);
			popup->setStyleSheet(
				"QListWidget { background-color: #21252E; color: #D0D3DC; border: 1px solid #2E3440; }"
				"QListWidget::item { padding: 6px 10px; }"
				"QListWidget::item:selected { background-color: #3A2B5A; color: #FFFFFF; }"
			);

			QObject::connect(popup, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
				int idx = popup->row(item);
				SelectIndex(idx);
				popup->hide();
				});
		}

		virtual ~ModernComboBoxWidget() override {
			delete popup;
		}

		// Items API
		void SetItems(const std::vector<std::string>& values) noexcept {
			items = values;
			if (currentIndex >= (int)items.size()) {
				currentIndex = -1;
				currentText.clear();
			}
			RebuildPopup();
			update();
		}

		void AddItem(const std::string& item) noexcept {
			items.push_back(item);
			RebuildPopup();
			update();
		}

		std::vector<std::string> GetItems() const noexcept {
			return items;
		}

		void SetSelectedIndex(int index) noexcept {
			SelectIndex(index);
		}

		void SetSelectedItem(const std::string& item) noexcept {
			auto it = std::find(items.begin(), items.end(), item);
			if (it != items.end()) {
				SelectIndex(static_cast<int>(std::distance(items.begin(), it)));
			}
		}

		int GetSelectedIndex() const noexcept { return currentIndex; }
		std::string GetSelectedItem() const noexcept { return currentText.toStdString(); }

		void SetPlaceholderText(const std::string& text) noexcept {
			placeholder = QString::fromStdString(text);
			update();
		}

		void SetOnSelectionChangedListener(const std::function<void(int)>& listener) noexcept {
			onIndexChanged = listener;
		}

		void SetOnSelectionChangedListener(const std::function<void(const std::string&)>& listener) noexcept {
			onTextChanged = listener;
		}

		// visual
		QSize sizeHint() const override {
			QFontMetrics fm(font());
			int w = fm.horizontalAdvance(currentText.isEmpty() ? placeholder : currentText) + 48;
			return QSize(w, 28);
		}

	protected:
		void paintEvent(QPaintEvent* /*evt*/) override {
			QPainter p(this);
			p.setRenderHint(QPainter::Antialiasing);

			// Background
			QColor bg = QColor("#1A1F2B");
			QColor border = QColor("#2E3440");
			QColor textColor = QColor("#D0D3DC");
			QColor arrowColor = QColor("#D0D3DC");
			QColor placeholderColor = QColor("#8B8F99");

			p.setPen(Qt::NoPen);
			p.setBrush(bg);
			p.drawRect(rect());

			// border line
			p.setPen(QPen(border, 1));
			p.drawRect(rect().adjusted(0, 0, -1, -1));

			// text
			QString display = currentText.isEmpty() ? placeholder : currentText;
			p.setPen(currentText.isEmpty() ? placeholderColor : textColor);
			QRect textRect(8, 0, width() - 32, height());
			p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, display);

			// arrow (chevron)
			QPoint center(width() - 16, height() / 2);
			QPolygon arrow;
			arrow << QPoint(center.x() - 6, center.y() - 2)
				<< QPoint(center.x() + 6, center.y() - 2)
				<< QPoint(center.x(), center.y() + 4);
			p.setBrush(arrowColor);
			p.setPen(Qt::NoPen);
			p.drawPolygon(arrow);
		}

		void mousePressEvent(QMouseEvent* event) override {
			if (event->button() != Qt::LeftButton) {
				QWidget::mousePressEvent(event);
				return;
			}
			TogglePopup();
		}

		void focusOutEvent(QFocusEvent* ev) override {
			QWidget::focusOutEvent(ev);
			// Hide popup when focus lost and popup not active
			if (popup && !popup->isActiveWindow()) {
				popup->hide();
			}
		}

	private:
		void RebuildPopup() {
			if (!popup) return;
			popup->clear();
			for (const auto& it : items) {
				popup->addItem(QString::fromStdString(it));
			}
		}

		void TogglePopup() {
			if (!popup) return;
			if (popup->isVisible()) {
				popup->hide();
				return;
			}

			RebuildPopup();
			// size calculations
			int itemHeight = 28;
			int maxVisible = 8;
			int visibleCount = std::min<int>((int)items.size(), maxVisible);
			int popupH = visibleCount * itemHeight + 2;
			int popupW = std::max(width(), 200);

			popup->setFixedWidth(popupW);
			popup->setFixedHeight(popupH);

			QPoint global = mapToGlobal(QPoint(0, height()));
			popup->move(global);
			if (currentIndex >= 0 && currentIndex < popup->count()) {
				popup->setCurrentRow(currentIndex);
				QListWidgetItem* it = popup->item(currentIndex);
				popup->scrollToItem(it, QAbstractItemView::PositionAtCenter);
			}
			popup->show();
			popup->setFocus();
		}

		void SelectIndex(int idx) {
			if (idx < 0 || idx >= (int)items.size()) {
				currentIndex = -1;
				currentText.clear();
			}
			else {
				currentIndex = idx;
				currentText = QString::fromStdString(items[idx]);
			}

			update();
			if (onIndexChanged) onIndexChanged(currentIndex);
			if (onTextChanged) onTextChanged(currentText.toStdString());
		}

	private:
		QListWidget* popup = nullptr;
		std::vector<std::string> items;
		QString currentText;
		QString placeholder;
		int currentIndex = -1;

		std::function<void(int)> onIndexChanged;
		std::function<void(const std::string&)> onTextChanged;
	};
}