/*
 *  AGL backend launcher
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * Copyright (C) 2016 EPAM Systems Inc.
 */

#include <QQuickWindow>
#include <QtCore/QCommandLineParser>
#include <QtCore/QUrlQuery>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQuickControls2/QQuickStyle>

#include <libhomescreen.hpp>
#include <qlibwindowmanager.h>

int main(int argc, char *argv[])
{
	QString myname = QString("android");

	QGuiApplication app(argc, argv);

	app.setApplicationName(myname);
	app.setApplicationVersion(QStringLiteral("0.1.0"));
	app.setOrganizationDomain(QStringLiteral("epam.com"));
	app.setOrganizationName(QStringLiteral("EPAM"));

	QQuickStyle::setStyle("AGL");

	QCommandLineParser parser;
	parser.addPositionalArgument("port", app.translate("main", "port for binding"));
	parser.addPositionalArgument("secret", app.translate("main", "secret for binding"));
	parser.addHelpOption();
	parser.addVersionOption();
	parser.process(app);
	QStringList positionalArguments = parser.positionalArguments();

	QQmlApplicationEngine engine;
	if (positionalArguments.length() != 2)
	{
		exit(EXIT_FAILURE);
	}
	int port = positionalArguments.takeFirst().toInt();
	QString secret = positionalArguments.takeFirst();

	QUrlQuery query;

	query.addQueryItem(QStringLiteral("token"), secret);

	QUrl bindingAddressWS;
	bindingAddressWS.setScheme(QStringLiteral("ws"));
	bindingAddressWS.setHost(QStringLiteral("localhost"));
	bindingAddressWS.setPort(port);
	bindingAddressWS.setPath(QStringLiteral("/api"));
	bindingAddressWS.setQuery(query);

	QQmlContext *context = engine.rootContext();

	context->setContextProperty(QStringLiteral("bindingAddressWS"), bindingAddressWS);

	std::string token = secret.toStdString();

	LibHomeScreen *hs = new LibHomeScreen();

	QLibWindowmanager *qwm = new QLibWindowmanager();

	// WindowManager
	if (qwm->init(port, secret) != 0)
	{
		exit(EXIT_FAILURE);
	}

	AGLScreenInfo screenInfo(qwm->get_scale_factor());

	// Request a surface as described in layers.json windowmanager’s file
	if (qwm->requestSurface(myname) != 0)
	{
		exit(EXIT_FAILURE);
	}

	// Create an event callback against an event type. Here a lambda is called when SyncDraw event occurs
	qwm->set_event_handler(QLibWindowmanager::Event_SyncDraw, [qwm, myname](json_object *object) {
		qDebug("Surface %s got Event_SyncDraw\n", myname.toLatin1().data());
		qwm->endDraw(myname);
	});

	QDBusInterface dbus("com.epam.DisplayManager", "/com/epam/DisplayManager", "com.epam.DisplayManager.Control");

	qwm->set_event_handler(QLibWindowmanager::Event_Visible, [qwm, myname, &dbus](json_object *object) {
		qDebug("Surface %s got Event_Visible\n", myname.toLatin1().data());
		dbus.call(QDBus::CallMode::NoBlock, "userEvent", static_cast<uint32_t>(1));
	});

	qwm->set_event_handler(QLibWindowmanager::Event_Invisible, [qwm, myname, &dbus](json_object *object) {
		qDebug("Surface %s got Event_Invisible\n", myname.toLatin1().data());
		dbus.call(QDBus::CallMode::NoBlock, "userEvent", static_cast<uint32_t>(2));
	});

	qwm->set_event_handler(QLibWindowmanager::Event_Active, [qwm, myname, &dbus](json_object *object) {
		qDebug("Surface %s got Event_Active\n", myname.toLatin1().data());
		dbus.call(QDBus::CallMode::NoBlock, "userEvent", static_cast<uint32_t>(3));
	});

	qwm->set_event_handler(QLibWindowmanager::Event_Inactive, [qwm, myname, &dbus](json_object *object) {
		qDebug("Surface %s got Event_Inactive\n", myname.toLatin1().data());
		dbus.call(QDBus::CallMode::NoBlock, "userEvent", static_cast<uint32_t>(4));
	});

	// HomeScreen
	hs->init(port, token.c_str());

	// Set the event handler for Event_TapShortcut which will activate the surface for windowmanager
	hs->set_event_handler(LibHomeScreen::Event_TapShortcut, [qwm, myname](json_object *object) {
		json_object *appnameJ = nullptr;

		if (json_object_object_get_ex(object, "application_name", &appnameJ))
		{
			const char *appname = json_object_get_string(appnameJ);

			if (myname == appname)
			{
				qDebug("Surface %s got tapShortcut\n", appname);
				qwm->activateSurface(myname);
			}
		}
	});

	engine.load(QUrl(QStringLiteral("qrc:/agl_surface_switcher.qml")));
	QObject *root = engine.rootObjects().first();

	QQuickWindow *window = qobject_cast<QQuickWindow *>(root);
	QObject::connect(window, SIGNAL(frameSwapped()), qwm, SLOT(slotActivateSurface()));

	return app.exec();
}
