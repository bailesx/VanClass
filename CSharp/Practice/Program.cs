// 네임 스페이스 시스템 사용
using System;
// Generic Collection을 정의하는 Class와 Interface
using System.Collections.Generic;
// LINQ를 사용하는 쿼리를 지원하는 Class와 Interface
using System.Linq;
using System.Text;

// namespace : 구조체, 클래스, 델리케이트, 인터페이스 등을 그룹화한 것/
namespace Practice
{
    // class : 크게 field와 method로 분류되며 각 정적인 상태와 동적인 기능을 표현함
    //         이는 독립적으로 존재할 수 있는 최소 단위이자 C#을 구성하는 기본 단위
    //         하나의 클래스를 통해 여러 개의 객체 생성 가능
    class Program
    {
        // Main method : 프로그램의 최초 진입점
        static void Main(string[] args)
        {
            // 비개행
            Console.Write("Hello World!");
            // 자동 개행
            Console.WriteLine("Hello World!");

            // 정수 자료형
            byte a = 255;
            sbyte b = 127;
            short c = 32767;
            ushort d = 65535;
            int e = 2147483647;
            uint f = 4294967295;
            long g = 922337203685477508;
            ulong h = 18446744073709551615;
            // 실수 자료형
            float i = 0.7f;
            double j = 0.16;
            decimal k = 0.29m;
            // 논리 자료형
            bool l = false;
            // 문자 자료형
            char m = '\0';
            string n = "hello world";
            // 이하 객체(object), 구조체(struct), 열거형(enum)이 존재
        }
    }
}
