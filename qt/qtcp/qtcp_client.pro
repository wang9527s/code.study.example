QT += core network
QT -= gui

CONFIG += c++11

TARGET = qtcp_client
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

HEADERS += \
    tcpclient.hpp

