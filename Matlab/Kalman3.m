%清除
clear all
clc;
close all;

%打开文件
[fname, fpath] = uigetfile(...
  {'*.txt', '*.*'}, ...
  'Pick a file');
FullData=textread(fullfile(fpath, fname), '' , 'headerlines', 6);   %读取数据的时跳过前0行
m_cwnd=FullData(:,4);    %取第一列窗口大小  
[r,c]=size(m_cwnd);
length=r;

%绘图
t = 1:length;
figure(1);
set(gcf,'position',[700 200 700 350])   %x y 长 宽
set(gcf,'color','w')
hold on
h1 = plot(t, m_cwnd, 'r');
xlabel('时间（×10ms）');
hold off
ylabel('窗口数（个）');
%axis(gca,[0,lcw_length,30,70]);%规定画图的范围