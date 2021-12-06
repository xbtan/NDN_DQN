%���
clear all
clc;
close all;

%���ļ�
[fname, fpath] = uigetfile(...
  {'*.txt', '*.*'}, ...
  'Pick a file');
FullData=textread(fullfile(fpath, fname), '' , 'headerlines', 6);   %��ȡ���ݵ�ʱ����ǰ0��
m_cwnd=FullData(:,4);    %ȡ��һ�д��ڴ�С  
[r,c]=size(m_cwnd);
length=r;

%��ͼ
t = 1:length;
figure(1);
set(gcf,'position',[700 200 700 350])   %x y �� ��
set(gcf,'color','w')
hold on
h1 = plot(t, m_cwnd, 'r');
xlabel('ʱ�䣨��10ms��');
hold off
ylabel('������������');
%axis(gca,[0,lcw_length,30,70]);%�涨��ͼ�ķ�Χ