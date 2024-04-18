QT += core network
QT -= gui

CONFIG += c++11

TARGET = qtcp_server
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

HEADERS += \
    tcpserver.hpp
