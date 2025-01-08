#include<iostream>
using namespace std;
 void nhap(int a[], int n)
 {
 	for(int i=0;i<n;i++)
 	{
 		cout<<"Nhap gia tri a["<<i<<"] = ";
 		cin>>a[i];
 	}
 }
 
  void xuat(int a[], int n)
 {
 	cout<<"Mang la :"<<endl;
 	for(int i=0;i<n;i++)
 	{
 		cout<<a[i]<<"  ";
 	}
 }
 
 int main()
 {
 	int n;
 	cout<<"Nhap so phan tu cua mang "<<endl;
 	cin>>n;
 	int a[n];
 	
 	nhap(a,n);
 	xuat(a,n);
 	
 	int tong =0;
 	for(int i=0;i<n;i++)
 	{
 		tong+=a[i];
 	}
 	
 	cout<<"Tong cua mang la "<<tong<<endl;
 	cout<<"Trung binh cong mang la "<<tong*1.0/n<<endl;
 	return 0;
 }
