# match.es -- demonstrate match command
for (subject = (foo bar buzz 42)) {
        match $subject (
                foo             {echo 'matched foo'}
                bar             {echo 'matched bar'}
                ??zz            {echo 'matched fizz/buzz'}
                [0-9]*          {echo 'matched number'}
                *               {echo 'default case'}
        )
}
