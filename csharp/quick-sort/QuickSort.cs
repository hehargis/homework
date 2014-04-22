using System;
using System.Collections;


namespace QuickSortDemo
{
    static class DataScale
    {
        public const int Maximum = 1000;
        public const int Minimum = 5;
        public const int Default = 50;
    }

    public interface ISortAlgorithm
    {
        int Scale { get; }

        void Seed();
        void Sort();
        IEnumerable Current();
    }

    public class QuickSort: ISortAlgorithm
    {
        public int Scale { get; private set; }
        private double[] Data;

        public QuickSort()
        {
            Initial(DataScale.Default);
        }

        public QuickSort(int scale)
        {
            Initial(scale);
        }

        public void Seed()
        {
            Random random = new Random();

            for (int i = 0; i < Scale; i++)
            {
                Data[i] = random.NextDouble();
            }
        }

        public void Sort()
        {
            DoSort(0, Scale - 1);
        }

        public IEnumerable Current()
        {
            for (int i = 0; i < Scale; i++)
            {
                yield return Data[i];
            }
        }

        private void Initial(int scale)
        {
            if (scale > DataScale.Maximum)
            {
                scale = DataScale.Maximum;
                System.Console.WriteLine("Scale too large, change to {0}", scale);
            }
            if (scale < DataScale.Minimum)
            {
                scale = DataScale.Minimum;
                System.Console.WriteLine("Scale too small, change to {0}", scale);
            }

            Scale = scale;
            Data = new double[Scale];
        }

        private void DoSort(int l, int r)
        {
            if (l >= r)
            {
                return;
            }

            int part = Partition(l, r);
            if (part > l)
            {
                DoSort(l, part - 1);
            }
            if (part < r)
            {
                DoSort(part + 1, r);
            }
        }

        private int Partition(int l, int r)
        {
            int p = (l + r) / 2;
            Swap(l, p);

            for (p = l, l = l + 1; l <= r;)
            {
                if (Data[l] > Data[p])
                {
                    Swap(l, r);
                    r = r - 1;
                }
                else
                {
                    l = l + 1;
                }
            }

            Swap(p, r);

            return r;
        }

        private void Swap(int a, int b)
        {
            double temp = Data[a];
            Data[a] = Data[b];
            Data[b] = temp;
        }
    }

    public class Demo
    {
        public static void Main()
        {
            QuickSort qs = new QuickSort(10);

            qs.Seed();
            qs.Sort();

            foreach (double i in qs.Current())
            {
                System.Console.Write(i + " ");
            }
            System.Console.WriteLine();
        }
    }
}