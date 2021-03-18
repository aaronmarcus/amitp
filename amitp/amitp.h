#pragma once

#ifndef AMITP_H
#define AMITP_H

#include "commonTools.h"
#include "Renderer.h" //jrtplib headers must be called before any other Windows headers to avoid redeclarations
#include "baseWindow.h"

#include <thread>
#include <chrono>

//main window class
class MainWindow : public BaseWindow<MainWindow>
{
public:
    PCWSTR  ClassName() const { return L"Sample Window Class"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void initiateWindow(int nCmdShow);

};

#endif